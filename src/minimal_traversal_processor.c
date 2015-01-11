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
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "minimal_traversal_processor.h"

uint nodeCount;

extern Transactionset ** srcPtrs;
extern Transactionset ** transPtrs;

extern Transaction backupInter;
extern Transaction leadInter;

uint getExploredNodesCount() {
	return nodeCount;
}

//Build the root Transactionset. The root Transactionset contains an array of
//children Transactionsets. Each child is a single-transactions Transactionset
//in the formal concept extent. For each single-transaction Transactionset is
//flagged as a generator or a non generator. A generator is flagged as forbidden
//and will later be ignored from the exploration process
void initialize(uint *items, uint itemsCount, Transactions * transactions,
		uint refCount, Transactionset * root) {

	AllocTranset * alloc;

	uint transactionIndex;

	Transaction * transactionsList;
	Transaction * currentTransaction;
	uint currentTransactionItemsCount;

	Transaction * currentIntersect;

	//counter for the available transactions
	uint i;

	//counter only for the transactions added
	uint j;

	Transactionset * elements;
	Transactionset * current;
	uint totalChildrenForbiddenCount;

	//processing
	transactionsList = transactions->encodedTransactions;

	alloc = popTranset();

	elements = alloc->transet;

	//all potential children are forbidden until proven to be non generators which
	//will cause this counter to be decremented
	totalChildrenForbiddenCount = itemsCount;

	for (i = 0, j = 0; i < itemsCount; i++) {

		//retrieving the object index
		transactionIndex = items[i];

		//Retrieving the itemset for that object
		currentTransaction = transactionsList + transactionIndex;

		//is a generator
		if (currentTransaction->itemCount == refCount) {
			continue;
		}

		totalChildrenForbiddenCount--;

		//setting a pointer on the next free transet memory space
		current = elements + j;

		//setting the suffix of the transactionset to be used later when building
		//this node previous siblings children
		current->transactions = transactionIndex;

		//initializing the node children
		current->children = NULL;
		//initializing the node children
		current->childrenCount = 0;

		currentTransactionItemsCount = currentTransaction->bufferSize;
		currentIntersect = &(current->intersect);
		currentIntersect->bufferSize = currentTransactionItemsCount;
		currentIntersect->limbCount = currentTransactionItemsCount;
		currentIntersect->firstSignificantLimb =
				currentTransaction->firstSignificantLimb;
		currentIntersect->itemCount = currentTransaction->itemCount;
		memcpy(currentIntersect->buffer, currentTransaction->buffer,
				sizeof(uint) * currentTransactionItemsCount);

		j++;
	}
	root->children = elements;
	root->alloc = alloc;
	root->childrenCount = j;

	//initialize the number of explored candidates
	nodeCount = j;
}

//Build the resulting intersection between Transaction * result and
//Transaction * operand and store the result in Transaction * result
void buildIntersection(Transaction * result, Transaction * left,
		Transaction * right) {

	uint intersectCount;
	uint res1stSigLimb;
	uint resLimbCount;
	uint *leftBuffer, *rightBuffer, *resBuffer;

	res1stSigLimb = max_sszt(left->firstSignificantLimb,
			right->firstSignificantLimb);
	resLimbCount = min_sszt(left->limbCount, right->limbCount);

	leftBuffer = left->buffer;
	rightBuffer = right->buffer;
	resBuffer = result->buffer;

	result->firstSignificantLimb = res1stSigLimb;
	result->limbCount = resLimbCount;

	intersectCount = 0;

	for (; res1stSigLimb < resLimbCount; res1stSigLimb++) {
		resBuffer[res1stSigLimb] = leftBuffer[res1stSigLimb]
				& rightBuffer[res1stSigLimb];
		intersectCount += __builtin_popcount(resBuffer[res1stSigLimb]);
	}

	result->itemCount = intersectCount;
}

//This recursive function is the main character in the exploration process
//Each node is breed with its (non-generators) forward sibling to generate its
//immediate children. Each child is then checked to determine whether it is a
//generator. This function is then called recursively to process each child.
void processRecursive(Transactionset * current, Transactions * transactions,
		uint refCount, uint level) {

	uint startIdx;

	//at the end this counter will equals the non generators count
	uint endIdx;

	Transaction * backupInterPtr;

	Transaction * leadInterPtr;

	Transaction * swapInterPrt;

	//declaration
	uint j, k, currentCount, variationValue;

	int i;

	Transactionset * currentChildren;
	Transactionset * leftElement;
	Transactionset * leftEltChildren;
	Transactionset * rightElement;
	Transactionset * leftEltCurrentChild;

	uint forbiddenLeftEltChildrenCount;
	uint nonGenCurrCnt;

	Transaction * leftEltIntersect;
	Transaction * leftEltCurChildIntersect;

	int counter;

	Transactionset * srcTranset;

	Transaction * srcIntersect;

	AllocTranset * alloc;

	//operations
	//get the current elment's children count
	currentCount = current->childrenCount;

	//current element children
	currentChildren = current->children;

	//initialize the reference array
	for (i = 0; i < currentCount; i++) {
		srcPtrs[i] = currentChildren + i;
	}

	qsort(srcPtrs, currentCount, sizeof(Transactionset*),
			compareTransetPtrByCardDesc);

	//loop through the first currentCount - 1 elements and cross them against
	//the following elements to form the elements children, forbidden elemnts
	//should be excluded
	//exclude forbidden elements
	k = currentCount - 1;

	//all elements from this level have the same items count which can be
	//Retrieved from the first element

	startIdx = 0;
	endIdx = currentCount - 1;
	backupInterPtr = &backupInter;
	leadInterPtr = &leadInter;

	//copy last non generator intersect buffer to the current node subintersect
	counter = k;

	//srcTranset = currentChildren + counter;
	srcTranset = srcPtrs[counter];

	srcIntersect = &(srcTranset->intersect);

	backupInter.limbCount = srcIntersect->limbCount;
	backupInter.bufferSize = srcIntersect->bufferSize;
	backupInter.firstSignificantLimb = srcIntersect->firstSignificantLimb;

	memcpy(backupInter.buffer, srcIntersect->buffer,
			srcIntersect->limbCount * sizeof(uint));

	//do not forget to update the counter which will be used in the next loop
	counter--;

	transPtrs[endIdx] = srcTranset;
	endIdx--;

	//flag forbidden non generators
	for (; counter >= 0; counter--) {

		srcTranset = srcPtrs[counter];
		//srcTranset = currentChildren + counter;

		buildIntersection(leadInterPtr, backupInterPtr,
				transactions->encodedTransactions + srcTranset->transactions);

		if (leadInterPtr->itemCount == refCount) {

			transPtrs[startIdx] = srcTranset;
			startIdx++;

		} else {

			swapInterPrt = leadInterPtr;
			leadInterPtr = backupInterPtr;
			backupInterPtr = swapInterPrt;

			transPtrs[endIdx] = srcTranset;
			endIdx--;
		}
	}

	//crossing session level
	for (i = 0; i < startIdx; i++) {

		//retrieve the current crossing element
		leftElement = transPtrs[i];

		alloc = popTranset();
		leftEltChildren = alloc->transet;

		//Retrieve the prefix of the next generation from the elements in the
		//current
		leftEltIntersect = &(leftElement->intersect);

		forbiddenLeftEltChildrenCount = 0;

		for (j = i + 1, nonGenCurrCnt = 0; j < currentCount; j++) {

			//debug variable

			rightElement = transPtrs[j];

			leftEltCurrentChild = leftEltChildren + nonGenCurrCnt;
			leftEltCurChildIntersect = &(leftEltCurrentChild->intersect);

			variationValue = rightElement->transactions;

			buildIntersection(leftEltCurChildIntersect, leftEltIntersect,
					transactions->encodedTransactions + variationValue);

			if (leftEltCurChildIntersect->itemCount == refCount) {
				forbiddenLeftEltChildrenCount++;
				continue;
			}

			nodeCount++;

			leftEltCurrentChild->transactions = variationValue;
			leftEltCurrentChild->children = NULL;
			leftEltCurrentChild->childrenCount = 0;

			nonGenCurrCnt++;
		}

		leftElement->children = leftEltChildren;
		leftElement->alloc = alloc;
		leftElement->childrenCount = nonGenCurrCnt;

		if (nodeCount < NODE_COUNT_THRESHOLD) {
			if (nonGenCurrCnt > 1) {
				processRecursive(leftElement, transactions, refCount, level + 1);
			}
		}

		pushTranset(leftElement->alloc);
	}
}

//Debug-only function
//Count all nodes in a Transactionset recursively
uint elementsCount(Transactionset * root) {

	Transactionset * children;
	Transactionset * currentChild;

	uint childrenCount;
	uint i;
	uint eltCount;

	children = root->children;
	childrenCount = root->childrenCount;

	eltCount = childrenCount;

	for (i = 0; i < childrenCount; i++) {
		currentChild = children + i;

		eltCount += elementsCount(currentChild);
	}

	return eltCount;
}

int compareTransetByCardAsc(const void * a, const void * b) {
	return (((Transactionset*) a)->intersect.itemCount
			- ((Transactionset*) b)->intersect.itemCount);
}

int compareTransetByCardDesc(const void * a, const void * b) {
	return (((Transactionset*) b)->intersect.itemCount
			- ((Transactionset*) a)->intersect.itemCount);
}

int compareTransetPtrByCardAsc(const void * a, const void * b) {
	return ((*(Transactionset**) a)->intersect.itemCount
			- (*(Transactionset**) b)->intersect.itemCount);
}

int compareTransetPtrByCardDesc(const void * a, const void * b) {
	return ((*(Transactionset**) b)->intersect.itemCount
			- (*(Transactionset**) a)->intersect.itemCount);
}

int compareConceptByProc(const void * a, const void * b) {
	return (((Concept*) b)->processed - ((Concept*) a)->processed);
}
