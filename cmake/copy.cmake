cmake_minimum_required(VERSION 3.24.0)
if(NOT SRC)
  message("SRC variable expected to set to source file path")
elseif(NOT DST)
  message("DST variable expected to set to destination directory path")
else()
  file(COPY ${SRC} DESTINATION ${DST})
endif()