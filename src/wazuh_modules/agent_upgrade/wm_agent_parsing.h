/*
 * Wazuh Module for Agent Upgrading
 * Copyright (C) 2015-2020, Wazuh Inc.
 * July 15, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#ifndef WM_AGENT_PARSING_H
#define WM_AGENT_PARSING_H

#include "wm_agent_upgrade.h"

/**
 * Parse received upgrade message, separetes it according to agent list
 * Will return response JSON to be returned to the socket
 * @param buffer message to be parsed
 * @return JSON with all the task responses
 * */
cJSON* wm_agent_parse_command(const char* buffer);

/**
 * Parses a response message based on state 
 * @param error_id 1 if error, 0 if successs
 * @param message response message
 * @param agent_id [OPTIONAL] id of the agent
 * @param task_id [OPTIONAL] id of the task
 * @return response json
 * */
cJSON* wm_agent_parse_response_mesage(int error_id, const char* message, const int *agent_id, const int* task_id);

/**
 * Parses a message to be sent to the request module
 * @param command task command
 * @param agent_id agent id
 * @return json to be sent
 * */
cJSON* wm_agent_parse_task_module_message(const char* command, const int agent_id);

/**
 * Parses upgrade command and returns an upgrade task from the information
 * Example: 
 * WPK Repository
 * { 
 *      "version"   : "3.12",
 *      "use_http"  : "false",
 *      "force_upgrade" : "0"
 * }
 * Custom WPK Package
 * { 
 *      "file_path" : "./wazuh_wpk"
 *      "installer" : "installer.sh"
 * }
 * @param params JSON where the task parameters are 
 * @param output message in case of error
 * @return upgrade task if there is no error, NULL otherwise
 * */
wm_upgrade_task* wm_agent_parse_upgrade_command(const cJSON* params, char* output);

#endif
