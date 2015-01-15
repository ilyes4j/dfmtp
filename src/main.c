/*
 Copyright (c) 2013, 2014, 2015, Mohamed Ilyes Dimassi.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
 must display the following acknowledgment:
 This product includes software developed by the Mohamed Ilyes Dimassi.
 4. Neither the name of the FST http://www.fst.rnu.tn/ nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY Mohamed Ilyes Dimassi ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL Mohamed Ilyes Dimassi BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include "transaction_loader.h"
#include "cli.h"
#include "minimal_traversal_processor.h"

int main(int argc, char *argv[]) {

	//--------------------------------------------------------------------------
	// Variables declaration
	//--------------------------------------------------------------------------

	//Performance profiling
	TIMESPEC start, end, diff;

	Transactionset * root;

	Transactions trans;
	Transactions * transPtr;

	char * selectedContextFile;

	uint exploredNodesCount;
	uint mtCount;

	uint *transet;
	uint transetSize;
	uint counter;

	//--------------------------------------------------------------------------
	// Processing
	//--------------------------------------------------------------------------
	transPtr = &trans;

	if (argc < 2) {
		invalidArguments("");
		exit(EXIT_FAILURE);
	}

	selectedContextFile = argv[1];

	printf("\nDEPTH-FIRST-MINIMAL-TRAVERSAL-MINER V1.0 Beta\n\n");

	printf("Using database %s\n", selectedContextFile);

	printf("\nLoading transactions...\n");

	loadDATContextFile(selectedContextFile, transPtr);

	printf("Transactions loaded\n");

	initTransetPool(transPtr->transactionsCount, transPtr->limbCount);

	printf("Output format :\n");
	printf("[visited][MTCount]\n\n");

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

	transetSize = transPtr->transactionsCount;
	transet = (uint *) malloc(transetSize * sizeof(uint));
	for (counter = 0; counter < transetSize; counter++) {
		transet[counter] = counter;
	}

	root = (Transactionset *) malloc(sizeof(Transactionset));

	initialize(transPtr, root);

	if (root->childrenCount > 1) {
		processRecursive(root, transPtr, 1);
	}

	exploredNodesCount = getExploredNodesCount();
	mtCount = getMinimalTraversalCount();

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	diff = diffTime(start, end);

	if (exploredNodesCount < NODE_COUNT_THRESHOLD) {

		printf("%10u; %10u; %3ld,%-5ld\n", exploredNodesCount, mtCount,
				diff.tv_sec, diff.tv_nsec / 10000);
	}

	pushTranset(root->alloc);
	free(root);

	freeTransetRepo(transPtr->transactionsCount);

	free(transPtr->transBuffArea);
	free(transPtr->encodedTransactions);

	printf("\nAll Concepts Processed [OK]\n");

	return EXIT_SUCCESS;
}
