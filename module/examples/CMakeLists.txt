build_lib_example(
  NAME tulpar-g0
  SOURCE_FILES tulpar-g0.cc
  LIBRARIES_TO_LINK ${libtulpar}
                    ${libcore}
)
