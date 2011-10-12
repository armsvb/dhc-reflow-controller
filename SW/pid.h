
#ifndef PID_H
#define PID_H


#define SCALING_FACTOR  2
;
typedef struct PID_DATA {
   int16_t lastError;
   int16_t p_term;
   int16_t i_term;
   int16_t d_term;
   int16_t P_Factor;
   int16_t I_Factor;
   int16_t D_Factor;
} pidData_t;


void pid_Init(int16_t p_factor, int16_t i_factor, int16_t d_factor, pidData_t *pid_st);

int16_t pid_Controller(int16_t setPoint, int16_t processValue, pidData_t *pid_st);

void pid_Reset_Integrator(pidData_t *pid_st);

#endif
