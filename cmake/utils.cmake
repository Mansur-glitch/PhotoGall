# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright © 2025 Mansur Mukhametzyanov
cmake_minimum_required(VERSION 3.24.0)
function(get_copy_script_cmd src_file dst_dir out_launch_cmd)
  set(copy_script "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/copy.cmake")
  set(copy_script_args -DSRC=${src_file} -DDST=${dst_dir})
  set(${out_launch_cmd} ${CMAKE_COMMAND} ${copy_script_args} -P ${copy_script} PARENT_SCOPE)
endfunction()
