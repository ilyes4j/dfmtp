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

	Concepts concepts;
	Concepts * conceptsPtr;

	Concept * conceptList;
	Concept * currentConcept;

	uint currCptItemsCnt;
	uint currCptTransCnt;
	uint conceptCounter;
	uint conceptListCount;

	char * selectedContextFile;
	char * selectedConceptsFile;

	uint exploredNodesCount;

	ssize_t toConcept;
	ssize_t fromConcept;

	//--------------------------------------------------------------------------
	// Processing
	//--------------------------------------------------------------------------
	transPtr = &trans;
	conceptsPtr = &concepts;

	if (argc < 2) {
		invalidArguments("");
		exit(EXIT_FAILURE);
	}

	selectedContextFile = argv[1];
	selectedConceptsFile = argv[2];
	fromConcept = strtol(argv[3], NULL, 10);
	toConcept = strtol(argv[4], NULL, 10);

	printf("\nDEPTH-FIRST-MINIMAL-TRAVERSAL-PROCESSOR V1.0 Beta\n\n");

	printf("Using database %s\n", selectedContextFile);
	printf("Using concepts %s\n", selectedConceptsFile);

	printf("\nLoading transactions...\n");

	loadDATContextFile(selectedContextFile, transPtr);

	printf("Transactions loaded\n");

	printf("\nLoading Formal Concepts...\n");

	loadLCMConceptsFile(selectedConceptsFile, conceptsPtr,
			transPtr->transactionsCount, transPtr->itemCount);

	printf("Formal Concepts Loaded\n\n");

	initTransetPool(transPtr->transactionsCount, transPtr->limbCount);

	printf("Output format :\n");
	printf("[Index][Extent][visited][approx][gain][Time][Stability][Done]\n\n");

	conceptList = concepts.concepts;
	conceptListCount = concepts.count;

	//if negative value it is considered an offset to the concept count
	toConcept = toConcept > 0 ? toConcept : conceptListCount + toConcept;
	//if negative value is way too big set it to end of the list
	toConcept = toConcept > 0 ? toConcept : conceptListCount;
	//if positive value out of bounds set to end of the list
	toConcept = toConcept > conceptListCount ? conceptListCount : toConcept;

	//if negative value it is considered an offset to the concept count
	fromConcept = fromConcept > 0 ? fromConcept : conceptListCount + fromConcept;
	//if negative value is way too big set to start of the list
	fromConcept = fromConcept > 0 ? fromConcept : 0;
	//if positive value out of bounds set to start of the list
	fromConcept = fromConcept < toConcept ? fromConcept : 0;

	printf("Found %u concepts available...\n", conceptListCount);
	printf("Processing concepts [%zd..%zd]...\n\n", fromConcept, toConcept - 1);

	for (conceptCounter = fromConcept; conceptCounter < toConcept;
			conceptCounter++) {

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

		currentConcept = conceptList + conceptCounter;
		currCptItemsCnt = currentConcept->itemsCount;
		currCptTransCnt = currentConcept->transactionsCount;

		root = (Transactionset *) malloc(sizeof(Transactionset));

		initialize(currentConcept->transactions, currCptTransCnt, transPtr,
				currCptItemsCnt, root);

		if (root->childrenCount > 1) {
			processRecursive(root, transPtr, currCptItemsCnt, 1);
		}

		exploredNodesCount = getExploredNodesCount();

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		diff = diffTime(start, end);

		if (exploredNodesCount < NODE_COUNT_THRESHOLD) {

			printf("%4u; %6u; %10u; %3ld,%-5ld;\n", conceptCounter, currCptTransCnt,
					exploredNodesCount, diff.tv_sec, diff.tv_nsec / 10000);
		}

		pushTranset(root->alloc);
		free(root);
	}

	freeTransetRepo(transPtr->transactionsCount);

	unloadConcepts(conceptsPtr);

	free(transPtr->transBuffArea);
	free(transPtr->encodedTransactions);

	printf("\nAll Concepts Processed [OK]\n");

	return EXIT_SUCCESS;
}
