/*
 *
 *  Created on: Aug 27, 2023
 *      Author: silas
 */

#ifndef HEAD_H_
#define HEAD_H_

#define MAXLEN 256

typedef struct CMD {
    const char name[MAXLEN];         //command name
    const char description[MAXLEN];  //command help menu text
    char output[MAXLEN];       //output of command as a string that can be changed
} CMD;

extern CMD aboutCMD;
extern CMD helpCMD;


#endif /* HEAD_H_ */
