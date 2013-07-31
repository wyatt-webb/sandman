#include "control.h"

#include <stdio.h>
#include <string.h>

#include "logger.h"
#include "timer.h"

#include "wiringPi.h"

// Constants
//

// Maximum duration of the moving state.
#define MAX_MOVING_STATE_DURATION_MS	(100 * 1000) // 100 sec.

// Maximum duration of the cool down state.
#define MAX_COOL_DOWN_STATE_DURATION_MS	(50 * 1000) // 50 sec.

// Time between commands.
#define COMMAND_INTERVAL_MS				(2 * 1000) // 2 sec.

// The pin to use for enabling controls.
#define ENABLE_GPIO_PIN				(7)

// Locals
//

// The names of the actions.
char const* const s_ControlActionNames[] =
{
	"stopped",		// ACTION_STOPPED
	"moving up",	// ACTION_MOVING_UP
	"moving down",	// ACTION_MOVING_DOWN
};

// The names of the states.
char const* const s_ControlStateNames[] =
{
	"idle",			// STATE_IDLE
	"moving up",	// STATE_MOVING_UP
	"moving down",	// STATE_MOVING_DOWN
	"cool down",	// STATE_COOL_DOWN
};

// Control members

unsigned int Control::ms_MovingDurationMS = MAX_MOVING_STATE_DURATION_MS;
unsigned int Control::ms_CoolDownDurationMS = MAX_COOL_DOWN_STATE_DURATION_MS;

// Functions
//

template<class T>
T const& Min(T const& p_A, T const& p_B)
{
	return (p_A < p_B) ? p_A : p_B;
}

// Control members

// Handle initialization.
//
// p_Name:			The name.
// p_UpGPIOPin:		The GPIO pin to use to move up.
// p_DownGPIOPin:	The GPIO pin to use to move down.
//
void Control::Initialize(char const* p_Name, int p_UpGPIOPin, int p_DownGPIOPin)
{
	// Copy the name.
	unsigned int const l_AmountToCopy = 
		Min(static_cast<unsigned int>(NAME_CAPACITY) - 1, strlen(p_Name));
	strncpy(m_Name, p_Name, l_AmountToCopy);
	m_Name[l_AmountToCopy] = '\0';
	
	m_State = STATE_IDLE;
	TimerGetCurrent(m_StateStartTime);
	m_DesiredAction = ACTION_STOPPED;

	// Setup the pins and set them low.
	m_UpGPIOPin = p_UpGPIOPin;
	pinMode(p_UpGPIOPin, OUTPUT);
	digitalWrite(p_UpGPIOPin, LOW);
	
	m_DownGPIOPin = p_DownGPIOPin;
	pinMode(p_DownGPIOPin, OUTPUT);
	digitalWrite(p_DownGPIOPin, LOW);
}

// Handle uninitialization.
//
void Control::Uninitialize()
{
	// Revert to input.
	pinMode(m_UpGPIOPin, INPUT);
	pinMode(m_DownGPIOPin, INPUT);
}

// Enable or disable all controls.
//
// p_Enable:	Whether to enable or disable all controls.
//
void Control::Enable(bool p_Enable)
{
	if (p_Enable == false)
	{
		// Revert to input.
		pinMode(ENABLE_GPIO_PIN, INPUT);

		LoggerAddMessage("Controls disabled.");
	}
	else
	{
		// Setup the pin and set it low.
		pinMode(ENABLE_GPIO_PIN, OUTPUT);
		digitalWrite(ENABLE_GPIO_PIN, LOW);

		LoggerAddMessage("Controls enabled.");
	}
}

// Set the durations.
//
// p_MovingDurationMS:		Duration of the moving state (in milliseconds).
// p_CoolDownDurationMS:	Duration of the cool down state (in milliseconds).
//
void Control::SetDurations(unsigned int p_MovingDurationMS, unsigned int p_CoolDownDurationMS)
{
	ms_MovingDurationMS = p_MovingDurationMS;
	ms_CoolDownDurationMS = p_CoolDownDurationMS;
	
	LoggerAddMessage("Control durations set to moving - %i ms, cool down - %i ms.", p_MovingDurationMS, p_CoolDownDurationMS);
}

// Process a tick.
//
void Control::Process()
{
	// Handle state transitions.
	switch (m_State) 
	{
		case STATE_IDLE:
		{
			// Wait until moving is desired to transition.
			if (m_DesiredAction == ACTION_STOPPED) {
				break;
			}

			// Transition to moving.
			if (m_DesiredAction == ACTION_MOVING_UP)
			{
				m_State = STATE_MOVING_UP;

				// Set the pin high.
				digitalWrite(m_UpGPIOPin, HIGH);
			}
			else
			{
				m_State = STATE_MOVING_DOWN;

				// Set the pin high.
				digitalWrite(m_DownGPIOPin, HIGH);
			}
			
			// Record when the state transition timer began.
			TimerGetCurrent(m_StateStartTime);

			LoggerAddMessage("Control \"%s\": State transition from \"%s\" to \"%s\" triggered.", 
				m_Name, s_ControlStateNames[STATE_IDLE], s_ControlStateNames[m_State]);
		}
		break;

		case STATE_MOVING_UP:
		{
			// Get elapsed time since state start.
			Time l_CurrentTime;
			TimerGetCurrent(l_CurrentTime);

			float l_ElapsedTimeMS = TimerGetElapsedMilliseconds(m_StateStartTime, l_CurrentTime);

			// Wait until moving up isn't desired or the time limit has run out.
			if ((m_DesiredAction == ACTION_MOVING_UP) && (l_ElapsedTimeMS < ms_MovingDurationMS))
			{
				break;
			}

			if (m_DesiredAction == ACTION_MOVING_DOWN)
			{
				// Transition to moving down.
				m_State = STATE_MOVING_DOWN;

				// Flip the pins.
				digitalWrite(m_UpGPIOPin, LOW);
				digitalWrite(m_DownGPIOPin, HIGH);
			}
			else
			{
				// Transition to cool down.
				m_State = STATE_COOL_DOWN;

				// Set the pins low.
				digitalWrite(m_UpGPIOPin, LOW);
				digitalWrite(m_DownGPIOPin, LOW);
			}
			
			// Record when the state transition timer began.
			TimerGetCurrent(m_StateStartTime);

			LoggerAddMessage("Control \"%s\": State transition from \"%s\" to \"%s\" triggered.", 
				m_Name, s_ControlStateNames[STATE_MOVING_UP], s_ControlStateNames[m_State]);
		}
		break;

		case STATE_MOVING_DOWN:
		{
			// Get elapsed time since state start.
			Time l_CurrentTime;
			TimerGetCurrent(l_CurrentTime);

			float l_ElapsedTimeMS = TimerGetElapsedMilliseconds(m_StateStartTime, l_CurrentTime);

			// Wait until moving down isn't desired or the time limit has run out.
			if ((m_DesiredAction == ACTION_MOVING_DOWN) && (l_ElapsedTimeMS < ms_MovingDurationMS))
			{
				break;
			}

			if (m_DesiredAction == ACTION_MOVING_UP)
			{
				// Transition to moving up.
				m_State = STATE_MOVING_UP;

				// Flip the pins.
				digitalWrite(m_UpGPIOPin, HIGH);
				digitalWrite(m_DownGPIOPin, LOW);
			}
			else
			{
				// Transition to cool down.
				m_State = STATE_COOL_DOWN;

				// Set the pins low.
				digitalWrite(m_UpGPIOPin, LOW);
				digitalWrite(m_DownGPIOPin, LOW);
			}
			
			// Record when the state transition timer began.
			TimerGetCurrent(m_StateStartTime);

			LoggerAddMessage("Control \"%s\": State transition from \"%s\" to \"%s\" triggered.", 
				m_Name, s_ControlStateNames[STATE_MOVING_DOWN], s_ControlStateNames[m_State]);
		}
		break;

		case STATE_COOL_DOWN:
		{
			// Clear the desired action.
			m_DesiredAction = ACTION_STOPPED;

			// Get elapsed time since state start.
			Time l_CurrentTime;
			TimerGetCurrent(l_CurrentTime);

			float l_ElapsedTimeMS = TimerGetElapsedMilliseconds(m_StateStartTime, l_CurrentTime);

			// Wait until the time limit has run out.
			if (l_ElapsedTimeMS < ms_CoolDownDurationMS)
			{
				break;
			}

			// Transition to idle.
			m_State = STATE_IDLE;

			// Set the pins low.
			digitalWrite(m_UpGPIOPin, LOW);
			digitalWrite(m_DownGPIOPin, LOW);

			LoggerAddMessage("Control \"%s\": State transition from \"%s\" to \"%s\" triggered.", 
				m_Name, s_ControlStateNames[STATE_COOL_DOWN], s_ControlStateNames[m_State]);
		}
		break;

		default:
		{
			LoggerAddMessage("Control \"%s\": Unrecognized state %d in Process()", m_State, m_Name);
		}
		break;
	}
}

// Set the desired action.
//
// p_DesiredAction:	The desired action.
//
void Control::SetDesiredAction(Actions p_DesiredAction)
{
	m_DesiredAction = p_DesiredAction;

	LoggerAddMessage("Control \"%s\": Setting desired action to \"%s\".", m_Name, 
		s_ControlActionNames[p_DesiredAction]);
}

