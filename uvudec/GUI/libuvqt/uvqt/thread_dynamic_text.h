/*
UVNet Universal Decompiler (uvudec)
Copyright 2010 John McMaster <JohnDMcMaster@gmail.com>
Licensed under the terms of the LGPL V3 or later, see COPYING for details
*/

#ifndef UVQT_THREAD_DYNAMIC_TEXT_H
#define UVQT_THREAD_DYNAMIC_TEXT_H

/*
My attempt to do nifty multithreaded data syncrhronization

The problem:
-Render data in one thread
-Actual data is in another
-Avoid locking the engine in multiple threads
	GUI will freeze up if doing analysis option in other thread

The idea:
-Maintain two buffers
-One buffer is reserved to be activly displayed
-The second buffer is updated while the other is being displayed
-Buffers are then switched
-Each time we do a display request, we check the data for a change
-If they are the same, we do not issue an update event
	Otherwise an inifinite update loop would result
-Results in double render, is there a way we could do it only once?
Maybe we can setup some custom update() paint event?

We can only paint in the context of a paint event, so no funny tricks allowed either

What if instead we try to get the data directly, and resubmit update if its not currently availible?
*/
class UVQtThreadDynamicTextData : public UVQtDynamicTextData
{
public: 
};

#endif

