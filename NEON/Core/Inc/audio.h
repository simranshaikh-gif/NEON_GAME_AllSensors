#ifndef INC_AUDIO_H_
#define INC_AUDIO_H_

#include <stdint.h>

void Audio_Init(void);
void Audio_Update(void);

// Queue a sound (Non-blocking)
void Audio_PlayJump(void);
void Audio_PlayDuck(void);
void Audio_PlayCrash(void);

// Haptic
void Audio_HapticClick(void);

#endif /* INC_AUDIO_H_ */
