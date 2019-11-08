/*
 * states.h
 *
 *  Created on: Sep 22, 2018
 *      Author: shromonaghosh
 */

#ifndef STATES_H_
#define STATES_H_

#include <stdio.h>

typedef enum {
    OFF=0,
    DRIVE=1,
    BACK=2,
    LEFT=3,
    RIGHT=4,
    WAIT=5,
} states;

#endif /* STATES_H_ */