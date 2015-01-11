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
 must display the following acknowledgement:
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

#ifndef STABILITY_PROCESSOR_H_
#define STABILITY_PROCESSOR_H_

#include "transaction_loader.h"

void initialize(uint *items, uint itemsCount, mpz_t * genTotalCountGMP,
		mpz_t * genLocalCountGMP, mpz_t * rangeCountGMP,
		Transactions * transactions, uint refCount, Transactionset * root);

void processRecursive(Transactionset * current, mpz_t * genTotalCountGMP,
		mpz_t * genLocalCountGMP, mpz_t * nongenTotalCountGMP,
		mpz_t * nongenLocalCountGMP, mpz_t * rangeCountGMP,
		mpz_t * mpzUpperThreshold, Transactions * transactions, uint refCount,
		uint level);

uint elementsCount(Transactionset * root);

uint nonForbiddenElementsCount(Transactionset * root);

uint getExploredNodesCount();

uint getApproxExploredNodesCount();

void displayElements(Transactionset * root);

int compareTransetByCardDesc(const void * a, const void * b);

int compareTransetByCardAsc(const void * a, const void * b);

int compareTransetPtrByCardAsc(const void * a, const void * b);

int compareTransetPtrByCardDesc(const void * a, const void * b);

int compareConceptByProc(const void * a, const void * b);

void findToleranceRange(mpz_t * mpzTolRange, uint tolerance, size_t extentSize);

void findThreshold(mpz_t * mpzThreshold, mpz_t * mpzReverseUpperThreshold,
		uint extentSize, uint threshold, uint precision);

uint hasDoneApprox();

uint hasCrossedThreshold();

#endif
