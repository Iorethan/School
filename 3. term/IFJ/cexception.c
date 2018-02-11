/* ***********************
 * The license for this file is included at the bottom of the file
 * **********************/

#include "cexception.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
volatile CEXCEPTION_FRAME_T CExceptionFrames[CEXCEPTION_NUM_ID] = {{ 0 }};
#pragma GCC diagnostic pop

//------------------------------------------------------------------------------------------
//  Throw
//------------------------------------------------------------------------------------------
void Throw(CEXCEPTION_T ExceptionID)
{
    unsigned int MY_ID = CEXCEPTION_GET_ID;
    CExceptionFrames[MY_ID].Exception = ExceptionID;
    if (CExceptionFrames[MY_ID].pFrame)
    {
        longjmp(*CExceptionFrames[MY_ID].pFrame, 1);
    }
    CEXCEPTION_NO_CATCH_HANDLER(ExceptionID);
}

//------------------------------------------------------------------------------------------
//  Explanation of what it's all for:
//------------------------------------------------------------------------------------------
/*
#define Try
    {                                                                   <- give us some local scope.  most compilers are happy with this
        jmp_buf *PrevFrame, NewFrame;                                   <- prev frame points to the last try block's frame.  new frame gets created on stack for this Try block
        unsigned int MY_ID = CEXCEPTION_GET_ID;                         <- look up this task's id for use in frame array.  always 0 if single-tasking
        PrevFrame = CExceptionFrames[CEXCEPTION_GET_ID].pFrame;         <- set pointer to point at old frame (which array is currently pointing at)
        CExceptionFrames[MY_ID].pFrame = &NewFrame;                     <- set array to point at my new frame instead, now
        CExceptionFrames[MY_ID].Exception = CEXCEPTION_NONE;            <- initialize my exception id to be NONE
        if (setjmp(NewFrame) == 0) {                                    <- do setjmp.  it returns 1 if longjump called, otherwise 0
            if (&PrevFrame)                                             <- this is here to force proper scoping.  it requires braces or a single line to be but after Try, otherwise won't compile.  This is always true at this point.

#define Catch(e)
            else { }                                                    <- this also forces proper scoping.  Without this they could stick their own 'else' in and it would get ugly
            CExceptionFrames[MY_ID].Exception = CEXCEPTION_NONE;        <- no errors happened, so just set the exception id to NONE (in case it was corrupted)
        }
        else                                                            <- an exception occurred
        { e = CExceptionFrames[MY_ID].Exception; e=e;}                  <- assign the caught exception id to the variable passed in.
        CExceptionFrames[MY_ID].pFrame = PrevFrame;                     <- make the pointer in the array point at the previous frame again, as if NewFrame never existed.
    }                                                                   <- finish off that local scope we created to have our own variables
    if (CExceptionFrames[CEXCEPTION_GET_ID].Exception != CEXCEPTION_NONE)  <- start the actual 'catch' processing if we have an exception id saved away
 */

/* ******************************************
 Copyright (c) 2007-2014 Mark VanderVoord

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 The end-user documentation included with the redistribution, if
 any, must include the following acknowledgment: "This product
 includes software developed for the CEXCeption Project, by Mark
 VanderVoord and other contributors", in the same place and form
 as other third-party acknowledgments.  Alternately, this
 acknowledgment may appear in the software itself, in the same
 form and location as other such third-party acknowledgments.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
 *******************************************/
