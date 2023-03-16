#pragma once

#ifndef TAU_IR_ALLOCATION_SAFETY_LEVEL
  #if _DEBUG
    #define TAU_IR_ALLOCATION_SAFETY_LEVEL 2
  #else
    #define TAU_IR_ALLOCATION_SAFETY_LEVEL 0
  #endif
#endif

#if TAU_IR_ALLOCATION_SAFETY_LEVEL == 0
  #define TAU_IR_ALLOCATION_TRACKING AllocationTracking::None
#elif TAU_IR_ALLOCATION_SAFETY_LEVEL == 1
  #define TAU_IR_ALLOCATION_TRACKING AllocationTracking::Count
#else
  #define TAU_IR_ALLOCATION_TRACKING AllocationTracking::DoubleDeleteCount
#endif

#ifndef TAU_IR_DEBUG_TYPES
  #ifdef _DEBUG
    #define TAU_IR_DEBUG_TYPES 1
  #else
    #define TAU_IR_DEBUG_TYPES 0
  #endif
#endif

