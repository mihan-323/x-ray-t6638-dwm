#ifndef xrRender_console_defsH
#define xrRender_console_defsH
#pragma once

extern ENGINE_API BOOL need_rsStaticSun;

// static sun & dynamic lights & deffered
#define IT_DEFFER_D_MODE need_rsStaticSun

// full dynamic & deffered
#define IT_DEFFER_F_MODE !need_rsStaticSun

#define $bit$ 1u << // Set current bit [0..x]

#endif
