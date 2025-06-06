# ##############################################################################
# arch/x86_64/src/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more contributor
# license agreements.  See the NOTICE file distributed with this work for
# additional information regarding copyright ownership.  The ASF licenses this
# file to you under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License.  You may obtain a copy of
# the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations under
# the License.
#
# ##############################################################################

add_subdirectory(${ARCH_SUBDIR})
add_subdirectory(${NUTTX_CHIP_ABS_DIR} EXCLUDE_FROM_ALL exclude_chip)
add_subdirectory(common)

# Include directories (before system ones) as PUBLIC so that it can be exposed
# to libboard
target_include_directories(arch BEFORE PUBLIC ${NUTTX_CHIP_ABS_DIR} common
                                              ${ARCH_SUBDIR})

if(NOT CONFIG_BUILD_FLAT)
  target_include_directories(arch_interface BEFORE PUBLIC ${NUTTX_CHIP_ABS_DIR}
                                                          common ${ARCH_SUBDIR})
endif()
