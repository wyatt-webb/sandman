#include "control.h"

#include <stdio.h>

#include "logger.h"
#include "serial_connection.h"
#include "timer.h"

// Constants
//

// Maximum duration of the moving state.
#define MAX_MOVING_STATE_DURATION_MS	(100 * 1000) // 100 sec.

// Maximum duration of the cool down state.
#define MAX_COOL_DOWN_STATE_DURATION_MS	(50 * 1000) // 50 sec.

// Time between commands.
#define COMMAND_INTERVAL_MS				(2 * 1000) // 2 sec.

// Function
//

// Control members

// Handle initialization.
//
// p_SerialConn:	The serial connection to the micro.
// p_CommandString:	The command string to send to the micro.
//
void Control::Initialize(SerialConnection* p_SerialConn, char const* p_CommandString)
{
	m_State = CONTROL_STATE_IDLE;
	m_StateStartTimeTicks = 0;
	m_MovingDesired = false;
	m_SerialConn = p_SerialConn;
	m_CommandString = p_CommandString;
	m_LastCommandTimeTicks = TimerGetTicks();
}

// Process a tick.
//
void Control::Process()
{
	// Handle state transitions.
	switch (m_State) 
	{
		case CONTROL_STATE_IDLE:
		{
			// Wait until moving is desired to transition.
			if (m_MovingDesired != true) {
				break;
			}

			// Transition to moving.
			m_State = CONTROL_STATE_MOVING;

			// Record when the state transition timer began.
			m_StateStartTimeTicks = TimerGetTicks();

			LoggerAddMessage("Control @ 0x%x: State transition from idle to moving triggered.", 
				reinterpret_cast<unsigned int>(this));           
		}
		break;

		case CONTROL_STATE_MOVING:
		{
			// Get elapsed time since state start.
			__int64 l_CurrentTimeTicks = TimerGetTicks();

			float l_ElapsedTimeMS = TimerGetElapsedMilliseconds(m_StateStartTimeTicks, l_CurrentTimeTicks);

			// Wait until moving isn't desired or the time limit has run out.
			if ((m_MovingDesired == true) && (l_ElapsedTimeMS < MAX_MOVING_STATE_DURATION_MS))
			{
				break;
			}

			// Transition to cool down.
			m_State = CONTROL_STATE_COOL_DOWN;

			// Record when the state transition timer began.
			m_StateStartTimeTicks = TimerGetTicks();

			LoggerAddMessage("Control @ 0x%x: State transition from moving to cool down triggered.",
				reinterpret_cast<unsigned int>(this));                      
		}
		break;

		case CONTROL_STATE_COOL_DOWN:
		{
			// Clear moving desired.
			m_MovingDesired = false;

			// Get elapsed time since state start.
			__int64 l_CurrentTimeTicks = TimerGetTicks();

			float l_ElapsedTimeMS = TimerGetElapsedMilliseconds(m_StateStartTimeTicks, l_CurrentTimeTicks);

			// Wait until the time limit has run out.
			if (l_ElapsedTimeMS < MAX_COOL_DOWN_STATE_DURATION_MS)
			{
				break;
			}

			// Transition to idle.
			m_State = CONTROL_STATE_IDLE;

			LoggerAddMessage("Control @ 0x%x: State transition from cool down to idle triggered.",
				reinterpret_cast<unsigned int>(this));                                 
		}
		break;

		default:
		{
			LoggerAddMessage("Control @ 0x%x: Unrecognized state %d in Process()", m_State, reinterpret_cast<unsigned int>(this));                      
		}
		break;
	}

	// Manipulate micro based on state.
	if ((m_SerialConn == NULL) || (m_CommandString == NULL))
	{
		return;
	}

	if (m_State == CONTROL_STATE_MOVING)
	{
		// Get elapsed time since last command.
		__int64 l_CurrentTimeTicks = TimerGetTicks();

		float l_ElapsedTimeMS = TimerGetElapsedMilliseconds(m_LastCommandTimeTicks, l_CurrentTimeTicks);

		// See if enough time has passed since last command.
		if (l_ElapsedTimeMS < COMMAND_INTERVAL_MS)
		{
			return;
		}

		unsigned long int l_NumBytesWritten = 0;
		m_SerialConn->WriteString(l_NumBytesWritten, m_CommandString);

		// Record when the last command was sent.
		m_LastCommandTimeTicks = TimerGetTicks();
	}
}

// Set moving desired for the next tick.
//
// p_MovingDesired:	Whether moving is desired.
//
void Control::SetMovingDesired(bool p_MovingDesired)
{
	m_MovingDesired = p_MovingDesired;

	LoggerAddMessage("Control @ 0x%x: Setting move desired to %d", reinterpret_cast<unsigned int>(this), p_MovingDesired);                      
}
