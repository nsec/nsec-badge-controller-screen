/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#if CONFIG_BADGE_MESH_ADMIN_COMMANDS

#ifdef __cplusplus
extern "C" {
#endif

void register_mesh_admin_commands(void);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_BADGE_MESH_ADMIN_COMMANDS */
