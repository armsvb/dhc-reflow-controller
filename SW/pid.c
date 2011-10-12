/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief General PID implementation for AVR.
 *
 * Discrete PID controller implementation. Set up by giving P/I/D terms
 * to Init_PID(), and uses a struct PID_DATA to store internal values.
 *
 * - File:               pid.c
 * - Compiler:           IAR EWAAVR 4.11A
 * - Supported devices:  All AVR devices can be used.
 * - AppNote:            AVR221 - Discrete PID controller
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support email: avr@atmel.com
 *
 * $Name$
 * $Revision: 456 $
 * $RCSfile$
 * $Date: 2006-02-16 12:46:13 +0100 (to, 16 feb 2006) $
 *****************************************************************************/
#include <inttypes.h>
#include <stdlib.h>

#include "pid.h"
#include "hw.h"

/*! \brief Initialisation of PID controller parameters.
 *
 *  Initialise the variables used by the PID algorithm.
 *
 *  \param p_factor  Proportional term.
 *  \param i_factor  Integral term.
 *  \param d_factor  Derivate term.
 *  \param pid  Struct with PID status.
 */
void pid_Init(int16_t p_factor, int16_t i_factor, int16_t d_factor, pidData_t *pid_st)
// Set up PID controller parameters
{
  // Start values for PID controller
  pid_st->i_term = 0;
  pid_st->lastError = 0;
  // Tuning constants for PID loop
  pid_st->P_Factor = p_factor;
  pid_st->I_Factor = i_factor;
  pid_st->D_Factor = d_factor;
  // Limits to avoid overflow
}


/*! \brief PID control algorithm.
 *
 *  Calculates output from setpoint, process value and PID status.
 *
 *  \param setPoint  Desired value.
 *  \param processValue  Measured value.
 *  \param pid_st  PID status struct.
 */
int16_t pid_Controller(int16_t setPoint, int16_t processValue, pidData_t *pid_st)
{
	int16_t error;
	int16_t out;

	error = setPoint - processValue;

  // Calculate Pterm and limit error overflow
	pid_st->p_term = pid_st->P_Factor * error;

  // Calculate Iterm and limit integral runaway
	if(abs(error) < AWR)
		pid_st->i_term += error/pid_st->I_Factor;
		
	if(pid_st->i_term > IMAX)
		pid_st->i_term = IMAX;
	else if (pid_st->i_term < IMIN)
		pid_st->i_term = IMIN;

  // Calculate Dterm
	pid_st->d_term = pid_st->D_Factor * (error - pid_st->lastError);

	pid_st->lastError = error;

	out = (pid_st->p_term + pid_st->i_term + pid_st->d_term);
	if(out > HMAX)
	{
		out = HMAX;
	}
	else if(out < HMIN)
	{
		out = HMIN;
	}

	return(out / SCALING_FACTOR);
}

/*! \brief Resets the integrator.
 *
 *  Calling this function will reset the integrator in the PID regulator.
 */
void pid_Reset_Integrator(pidData_t *pid_st)
{
  pid_st->i_term = 0;
}
