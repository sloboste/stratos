Solution to Stratos Embedded Development Challenge
Author: Steven Sloboda : sloboste@umich.edu

The current directory should contain the following files:
README.txt			:	This document
PROTOCOL.txt		:	Outlines the protocol for communication between the host
						processor and the client over the spi bus
API_COMMENTS.txt	:	Contains my thoughts and suggestions pertaining to the
						API given by "flash.h", "spi.h", and "swipe.h"
flash.h				: 	Header file provided by Stratos
flash.c				:	Dummy implementation of flash.h
spi.h				: 	Header file provided by Stratos
spi.c				:	Dummy implementation of spi.h
swipe.h				: 	Header file provided by Stratos
swipe.c				:	Dummy implementation of swipe.h
main.c				:	My solution to the challenge - contains the main loop
						code for the host processor
makefile			: 	A makefile for the project

