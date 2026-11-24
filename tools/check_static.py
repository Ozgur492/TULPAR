#!/usr/bin/env python3
# Copyright (c) 2026 TULPAR project team
# SPDX-License-Identifier: GPL-2.0-only
"""Static checks of the TULPAR agent code (gate G0 and every pull request).

Checks, with the requirement each one serves:
  harness     MD9 / RSD-SIM-02a  no agent file includes a harness or ground-truth header
  module      RSD Table 4.25     a module includes only its own, common and M-CRY headers
  lsc         MD19               the LSC includes and links no other agent module
  wall_clock  RSD-SIM-01a        no wall-clock or unseeded random call in agent code
  cmake       all of the above   the static targets link only what the rules allow
  routing     RSD-NET-01a        no ns-3 routing protocol is installed: every InternetStackHelper gets static
                                 routing with no route added, and no routing helper or route call appears

Exit status 0 when no rule is violated, 1 otherwise. Rules: tools/deps_rules.json.
"""
import argparse
import json
import re
import sys
from pathlib import Path

INCLUDE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]', re.M)
SOURCE_SUFFIXES = {'.h', '.hpp', '.cc', '.cpp'}


def strip_comments(text):
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)


def strip_strings(text):
    return re.sub(r'"(?:\\.|[^"\\\n])*"', '""', text)


class Checker:
    def __init__(self, root, rules):
        self.root = Path(root).resolve()
        self.rules = rules
        self.parts = {name: (self.root / rel).resolve() for name, rel in rules['parts'].items()}
        self.violations = []
        self.headers = {}
        for name, path in self.parts.items():
            for h in sorted(path.rglob('*.h')):
                self.headers.setdefault(h.name, []).append(name)

    def part_of(self, path):
        path = Path(path).resolve()
        for name, base in self.parts.items():
            if base == path or base in path.parents:
                return name
        return None

    def fail(self, rule, where, message):
        self.violations.append((rule, where, message))

    def rule_for(self, src_part, dst_part):
        if dst_part in ('sim',):
            return 'harness'
        if src_part == 'lsc':
            return 'lsc'
        if src_part == 'sec' and dst_part == 'mis':
            return 'sec_motion'
        return 'module'

    def check_file(self, path, part):
        text = path.read_text(encoding='utf-8', errors='replace')
        rel = path.relative_to(self.root)
        allowed = set(self.rules['may_include'][part])
        is_agent = part in self.rules['agent_parts']
        code = strip_comments(text)
        for m in INCLUDE.finditer(code):
            delim, name = m.group(1), m.group(2)
            line = code.count('\n', 0, m.start()) + 1
            where = f'{rel}:{line}'
            if path.stem == 'tulpar-mesh-core' and not (
                (delim == '<' and '/' not in name and name not in self.headers)
                or name == 'tulpar-mesh-core.h'
            ):
                self.fail('mesh_core', where, f'mesh core includes {name}')
            if name.startswith('ns3/'):
                if is_agent and name in self.rules['harness_ns3_headers']:
                    self.fail('harness', where, f'includes harness header {name}')
                continue
            targets = []
            if delim == '"':
                candidate = (path.parent / name).resolve()
                if candidate.exists():
                    targets = [self.part_of(candidate)]
                elif Path(name).name in self.headers and '/' not in name:
                    targets = self.headers[Path(name).name]
            elif Path(name).name in self.headers and name.startswith('tulpar'):
                targets = self.headers[Path(name).name]
            for dst in targets:
                if dst is None or dst in allowed:
                    continue
                self.fail(self.rule_for(part, dst), where, f'{part} includes {name} from {dst}')
        if is_agent:
            flat = strip_strings(code)
            for pattern in self.rules['wall_clock_patterns']:
                for m in re.finditer(pattern, flat):
                    line = flat.count('\n', 0, m.start()) + 1
                    self.fail('wall_clock', f'{rel}:{line}', f'matches {pattern!r}')

    def check_sources(self):
        count = 0
        for part, base in self.parts.items():
            for path in sorted(base.rglob('*')):
                if path.suffix in SOURCE_SUFFIXES:
                    self.check_file(path, part)
                    count += 1
        return count

    def check_cmake(self):
        cmake = self.root / 'module' / 'CMakeLists.txt'
        text = strip_cmake_comments(cmake.read_text(encoding='utf-8'))
        targets = self.rules['cmake_targets']
        declared = {}
        for pos, body in cmake_calls(text, 'tulpar_static_target'):
            tokens = body.split()
            name, deps, section = tokens[0], [], None
            for tok in tokens[1:]:
                if tok in ('SOURCES', 'INCLUDE', 'DEPS'):
                    section = tok
                elif section == 'DEPS':
                    deps.append(tok)
            names = [name]
            if '${m}' in name:
                loops = list(re.finditer(r'foreach\(\s*m\s+([^)]*)\)', text[:pos]))
                names = [name.replace('${m}', x) for x in loops[-1].group(1).split()] if loops else []
            for n in names:
                suffix = n.split('_', 1)[1] if '_' in n else n
                declared[n] = [d.replace('${m}', suffix) for d in deps]
        for n in sorted(set(targets) - set(declared)):
            self.fail('cmake', 'module/CMakeLists.txt', f'target {n} not found')
        for name, deps in declared.items():
            part = targets.get(name)
            if part is None:
                self.fail('cmake', 'module/CMakeLists.txt', f'unknown static target {name}')
                continue
            allowed = set(self.rules['may_include'][part])
            for d in deps:
                if d in ('tulpar', '${libtulpar}'):
                    self.fail('harness', 'module/CMakeLists.txt', f'{name} links the harness library')
                elif d in targets and targets[d] not in allowed:
                    self.fail(self.rule_for(part, targets[d]), 'module/CMakeLists.txt', f'{name} links {d}')
        if declared.get('tulpar_mesh_core'):
            self.fail('mesh_core', 'module/CMakeLists.txt', 'tulpar_mesh_core links another target')
        return declared


    def check_routing(self):
        """RSD-NET-01a: routing between nodes belongs to the overlay; IPv4 keeps only local delivery."""
        base = self.root / 'module'
        for path in sorted(base.rglob('*')):
            if path.suffix not in SOURCE_SUFFIXES:
                continue
            rel = path.relative_to(self.root)
            code = strip_comments(path.read_text(encoding='utf-8', errors='replace'))
            for pattern in self.rules['routing_patterns']:
                for m in re.finditer(pattern, code):
                    self.fail('routing', f'{rel}:{code.count(chr(10), 0, m.start()) + 1}', f'matches {pattern!r}')
            for m in re.finditer(r'\bInternetStackHelper\s+(\w+)\s*;', code):
                var, line = m.group(1), code.count('\n', 0, m.start()) + 1
                install = re.search(r'\b' + var + r'\s*\.\s*Install\s*\(', code[m.end():])
                static = re.search(r'\b' + var + r'\s*\.\s*SetRoutingHelper\s*\(\s*Ipv4StaticRoutingHelper\s*\(',
                                   code[m.end():])
                if install and not (static and static.start() < install.start()):
                    self.fail('routing', f'{rel}:{line}',
                              f'{var} is installed without SetRoutingHelper(Ipv4StaticRoutingHelper()) first')


def cmake_calls(text, func):
    """Yield (position, body) of every call of a CMake function, with balanced parentheses."""
    i = 0
    while True:
        j = text.find(func + '(', i)
        if j < 0:
            return
        k, depth = j + len(func) + 1, 1
        while depth and k < len(text):
            depth += {'(': 1, ')': -1}.get(text[k], 0)
            k += 1
        yield j, text[j + len(func) + 1:k - 1]
        i = k


def strip_cmake_comments(text):
    return re.sub(r'#[^\n]*', '', text)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument('--root', default=Path(__file__).resolve().parent.parent)
    ap.add_argument('--rules', default=None)
    args = ap.parse_args()
    root = Path(args.root).resolve()
    rules = json.loads(Path(args.rules or root / 'tools' / 'deps_rules.json').read_text(encoding='utf-8'))
    c = Checker(root, rules)
    n = c.check_sources()
    declared = c.check_cmake()
    c.check_routing()
    print(f'check_static: {n} source files, {len(declared)} static targets')
    if c.violations:
        for rule, where, message in c.violations:
            print(f'FAIL [{rule}] {rules["rule_ids"].get(rule, rule)} :: {where}: {message}')
        print(f'check_static: {len(c.violations)} violation(s)')
        return 1
    for rule, text in rules['rule_ids'].items():
        print(f'PASS [{rule}] {text}')
    return 0


if __name__ == '__main__':
    sys.exit(main())
