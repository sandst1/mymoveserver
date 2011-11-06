#!/bin/bash

awk '
    {
 	if (NR % 2 == 0)
	{
	
	    print $0 "0 0 0 0"
	}
        else
	{
	    print $0
	}

    }

' ann_training_data
