#pragma once

#include <obs-module.h>

#include "globals.h"

struct obs_source_info freetype2_source_info_v2;
struct obs_source_info freetype2_source_info_v1;
const char* ft2_source_get_name(void*);
void* ft2_source_create(obs_data_t*, obs_source_t*);
void ft2_source_destroy(void*);
void ft2_source_update(void*, obs_data_t*);
uint32_t ft2_source_get_width(void*);
uint32_t ft2_source_get_height(void*);
void ft2_source_render(void*, gs_effect_t*);
void ft2_video_tick(void*, float);
void ft2_source_defaults(obs_data_t*, int);
void ft2_source_defaults_v1(obs_data_t*);
void ft2_source_defaults_v2(obs_data_t*);
obs_properties_t* ft2_source_properties(void*);
