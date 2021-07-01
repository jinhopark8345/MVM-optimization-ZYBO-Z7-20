///
/******************************************************************************
Copyright (c) 2017 SoC Design Laboratory, Konkuk University, South Korea
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met: redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer;
redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution;
neither the name of the copyright holders nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Authors: Junsu Heo (junsuheo@konkuk.ac.kr)
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "parameter.h"
#include "benchmarking.h"


void mvm_ref(my_complex *out, my_complex *in, my_complex *h_inv)
{
	int n, i, j, k;
	for (n = 0; n < NOFDM; n++)
		for (i = 0; i < NSC; i++)
			for (j = 0; j < NTX; j++)
				for (k = 0; k < NRX; k++)
				{
					out[n*NSC*NRX + i * NRX + j].real += h_inv[i*NTX*NRX + j * NRX + k].real * in[n*NSC*NRX + k * NSC + i].real
						- h_inv[i*NTX*NRX + j * NRX + k].img * in[n*NSC*NRX + k * NSC + i].img;
					out[n*NSC*NRX + i * NRX + j].img += h_inv[i*NTX*NRX + j * NRX + k].img * in[n*NSC*NRX + k * NSC + i].real
						+ h_inv[i*NTX*NRX + j * NRX + k].real * in[n*NSC*NRX + k * NSC + i].img;
				}
}

void mvm_opt(my_complex *out, my_complex *in, my_complex *h_inv)
{
	///////////////////////
	// original code
	// Edit code below!! //
	/*
    int n, i, j, k;
	for (i = 0; i < NSC; i++)
		for (n = 0; n < NOFDM; n++)
			for (j = 0; j < NTX; j++)
				for (k = 0; k < NRX; k++)
				{
					out[n*NSC*NRX + i * NRX + j].real +=
							h_inv[i*NTX*NRX + j * NRX + k].real *
							in[n*NSC*NRX + k * NSC + i].real -
							h_inv[i*NTX*NRX + j * NRX + k].img *
							in[n*NSC*NRX + k * NSC + i].img;
					out[n*NSC*NRX + i * NRX + j].img +=
							h_inv[i*NTX*NRX + j * NRX + k].img *
							in[n*NSC*NRX + k * NSC + i].real+
							h_inv[i*NTX*NRX + j * NRX + k].real *
							in[n*NSC*NRX + k * NSC + i].img;
				}
				*/
	// Edit code above!! //
	///////////////////////

	/********************** for loop optimization start ************************/
	/*
	 * n, i, k, j : 110.255 //
	 * n, k, i, j : 217.365 //
	 * 	 	| n | i | j | k
	 * tmp1 | 3 | 2 | 1 | 0
	 * tmp2 | 0 | 2 | 2 | 1
	 * tmp3 | 3 | 1 | 0 | 3
	 *      | 6 | 5 | 3 | 4 : change of each variable, and finding time
	 * */
	// NSC = 2048
	// NRX, NTX = 8
	// NOFDM = 10
	// NSC * NRX = 2048 * 8 = 16384
	// NTX * NRX = 8 * 8 = 64

	/*
	int n, i, j, k;
	int tmp1, tmp2, tmp3;
	for (n = 0; n < NOFDM; n++) // n: 0 -> 10
		for (i = 0; i < NSC; i++) // i: 0 -> 2048
			for (k = 0; k < NRX; k++) // k: 0 -> 8
				for (j = 0; j < NTX; j++) // j: 0 -> 8
				{

					// tmp1 = n * 16384 + i * 8    + j // out(matrix)
					// tmp2 = i * 64    + j * 8    + k // h_inv(matrix)
					// tmp3 = n * 16384 + k * 2048 + i // in(matrix)

					//tmp1 = n*NSC*NRX + i * NRX + j;
					//tmp2 = i*NTX*NRX + j * NRX + k;
					//tmp3 = n*NSC*NRX + k * NSC + i;
					tmp1 = n * 16384 +                        i * 8    + j; // out(matrix)
					tmp2 =                        i * 64    + j * 8    + k; // h_inv(matrix)
					tmp3 = n * 16384 + k * 2048                        + i; // in(matrix)
					out[tmp1].real += h_inv[tmp2].real * in[tmp3].real -
									  h_inv[tmp2].img  * in[tmp3].img;
					out[tmp1].img  += h_inv[tmp2].img  * in[tmp3].real+
								      h_inv[tmp2].real * in[tmp3].img;
				}

	*/
	/********************** for loop optimization end ************************/

	/********************** for loop optimization & tiling start ************************/

	// tiling between k & j => doesn't get better
	// n, i, k, j : 120 ~ 130 // depends on tile size & how i implement it , tiling between k,j
	// NSC = 2048
	// NRX, NTX = 8
	// NOFDM = 10
	// NSC * NRX = 2048 * 8 = 16384
	// NTX * NRX = 8 * 8 = 64

	/*
	int n, i, j, k;
	int tmp1, tmp2, tmp3;


	int TILE_SIZE = 4;
	int kk, jj;

	for (n = 0; n < NOFDM; n++) // n: 0 -> 10
		for (i = 0; i < NSC; i++) // i: 0 -> 2048
			for (k = 0; k < NRX/TILE_SIZE; k++ ) // k: 0 -> 2
			{
				for (j = 0; j < NTX/TILE_SIZE; j++) // j: 0 -> 2
				{
					for(kk = k*TILE_SIZE ; kk < k * TILE_SIZE + TILE_SIZE; kk++) // kk: 0 -> 4, 4->8
					{
						for(jj = j * TILE_SIZE; jj < j * TILE_SIZE + TILE_SIZE; jj++)
						{
							// tmp1 = n * 16384 + i * 8    + j // out(matrix)
							// tmp2 = i * 64    + j * 8    + k // h_inv(matrix)
							// tmp3 = n * 16384 + k * 2048 + i // in(matrix)
							//tmp1 = n*NSC*NRX + i * NRX + j;
							//tmp2 = i*NTX*NRX + j * NRX + k;
							//tmp3 = n*NSC*NRX + k * NSC + i;

							tmp1 = n * 16384                       + i * 8    + jj; // out(matrix)
							tmp2 =                       i * 64    + jj * 8    + kk; // h_inv(matrix)
							tmp3 = n * 16384 + kk * 2048                       + i; // in(matrix)

							out[tmp1].real += h_inv[tmp2].real * in[tmp3].real -
											  h_inv[tmp2].img  * in[tmp3].img;
							out[tmp1].img  += h_inv[tmp2].img  * in[tmp3].real+
											  h_inv[tmp2].real * in[tmp3].img;

						}
					}
				}
			}

	*/

	// tiling between i & n => does! get better
	/*
	 * reasons:
	 * when n changes, column access is always twice(from very far)
	 * but when i changes, column access is usually twice but sometimes 3 times(when ii 7->8, it becomes column access as well)
	 * */
	// n, i, k, j
	// T = 2, 86.082
	// T = 4, 85.150
	// T = 8, 84.833

	// NSC = 2048
	// NRX, NTX = 8
	// NOFDM = 10
	// NSC * NRX = 2048 * 8 = 16384
	// NTX * NRX = 8 * 8 = 64

	int n, i, j, k;
	int tmp1, tmp2, tmp3;

	int ii;
	int T = 8;
	int cur_i;

	for (i = 0; i < NSC/T; i++) // i: 0 -> 2048/T
	{
		for (n = 0; n < NOFDM; n++) // n: 0 -> 10
		{
			cur_i = i * T;
			for(ii = cur_i ; ii < cur_i + T ; ii++) // ii: 0 -> T, T -> 2T, 2T -> 3T ...
			{
				for (k = 0; k < NRX; k++) // k: 0 -> 8
				{
					for (j = 0; j < NTX; j++) // j: 0 -> 8
					{
						/*
						tmp1 = n * 16384 +                        ii * 8    + j; // out(matrix)
						tmp2 =                        ii * 64    + j * 8    + k; // h_inv(matrix)
						tmp3 = n * 16384 + k * 2048                        + ii; // in(matrix)
						*/
						tmp1 = n * 16384 +                        ii * 8    + j; // out(matrix)
						tmp2 =                        ii * 64    + j * 8    + k; // h_inv(matrix)
						tmp3 = n * 16384 + k * 2048                        + ii; // in(matrix)
						out[tmp1].real += h_inv[tmp2].real * in[tmp3].real -
										  h_inv[tmp2].img  * in[tmp3].img;
						out[tmp1].img  += h_inv[tmp2].img  * in[tmp3].real+
										  h_inv[tmp2].real * in[tmp3].img;
					}
				}
			}
		}
	}



	return;
}

unsigned int initializor_dummy(my_complex* uiParam0, my_complex* uiParam1, my_complex* uiParam2) { return 1; }
unsigned int validator_dummy(my_complex* uiParam0, my_complex* uiParam1, my_complex* uiParam2) { return 1; }

int main()
{
	int i, mode_sel= 0;

	my_complex *outFFT = 0;
	my_complex *outMVM_ref = 0;
	my_complex *outMVM_opt = 0;
	my_complex *h_inv = 0;

	float error = 0;
	float signal = 0;
	float NSRdB = 0;
	double opt_time = 0;
	double ref_time = 0;
	BENCHMARK_CASE *pBenchmarkCase;
	BENCHMARK_STATISTICS    *pStat;

	outFFT = (my_complex*)malloc(sizeof(my_complex) * NOFDM * NRX * NSC); if (outFFT == NULL) { xil_printf("Memory allocation error\r\n"); };
	outMVM_ref = (my_complex*)calloc(NOFDM * NRX * NSC, sizeof(my_complex)); if (outMVM_ref == NULL) { xil_printf("Memory allocation error\r\n"); };
	outMVM_opt = (my_complex*)calloc(NOFDM * NRX * NSC, sizeof(my_complex)); if (outMVM_opt == NULL) { xil_printf("Memory allocation error\r\n"); };
	h_inv = (my_complex*)calloc(NTX * NRX * NSC, sizeof(my_complex)); if (h_inv == NULL) { xil_printf("Memory allocation error\r\n"); };

	srand(10);

	//Random input generation
	for (i = 0; i < NOFDM * NRX *NSC; i++)
	{
		outFFT[i].real = (float)(rand() % 10);
		outFFT[i].img = (float)(rand() % 10);

	}
	//Random H_inverse generation
	for (i = 0; i < NTX * NRX * NSC; i++)
	{
		h_inv[i].real = (float)(rand() % 10);
		h_inv[i].img = (float)(rand() % 10);
	}

	for (mode_sel = 0; mode_sel < _mode; mode_sel++)
	{
		if (mode_sel == 0)
				mvm_ref(&outMVM_ref[i*NRX*NSC], &outFFT[i*NRX*NSC], h_inv);
		else
				mvm_opt(&outMVM_opt[i*NRX*NSC], &outFFT[i*NRX*NSC], h_inv);
	}

	for (i = 0; i < NOFDM*NTX*NSC; i++) {
		error += pow((outMVM_ref[i].real - outMVM_opt[i].real), 2) + pow((outMVM_ref[i].img - outMVM_opt[i].img), 2);;
		signal += pow((outMVM_ref[i].real), 2) + pow((outMVM_ref[i].img), 2);
	}

	NSRdB = 10 * log10(error / signal);
	printf("\n\nMeasured Accuracy: NSR(dB) = %0.3f \n", NSRdB);


	BENCHMARK_CASE BenchmarkCases[TEST_CASE] = {
		{"MVM Reference", TEST_ROUNDS, initializor_dummy, mvm_ref,
			{outMVM_ref, outFFT,h_inv},
			0, validator_dummy
		},
		{"MVM Optimization", TEST_ROUNDS, initializor_dummy, mvm_opt,
			{outMVM_ref, outFFT,h_inv},
			0, validator_dummy
		}
	};

	Xil_L1DCacheEnable();
	Xil_L2CacheDisable();
	Xil_L1ICacheEnable();

	xil_printf("\r\n");
	xil_printf("-----Benchmarking Start-----\r\n");
	for (i = 0; i < TEST_CASE; i++)
	{
		pBenchmarkCase = &BenchmarkCases[i];
		pStat = &(pBenchmarkCase->stat);
		printf("Case %d: %s\r\n", i, pBenchmarkCase->pName);
		run_benchmark_single(pBenchmarkCase);
		statistics_print(pStat);
		if (i == 0)
			ref_time = pStat->ullMax;
		else
			opt_time = pStat->ullMax;
	}
	xil_printf("----Benchmarking Complete----\r\n");
	xil_printf("\r\n");
	printf("Optimized MVM is x%.2f faster than Reference\r\n", (double)(ref_time/opt_time));
	xil_printf("\r\n");

	free(outFFT);
	free(outMVM_ref);
	free(outMVM_opt);
	free(h_inv);




	return 0;
}




