/* (C) Copyright 1989- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/* WAM_FTZ - flush-to-zero / denormals-are-zero control for the wave model.
 *
 * In single precision, spectral values near the noise floor turn parts of
 * SNONLIN/IMPLSCH into subnormal arithmetic; on x86 every such operation
 * takes a microcode assist (~100x slower).  Intel compilers enable FTZ/DAZ
 * process-wide by default; GNU does not.  WAVEMDL therefore sets the two
 * MXCSR bits on entry and restores the caller's value on exit, on every
 * OpenMP thread, so the host model (e.g. IFS) is unaffected.
 *
 * Measured (IFS CY50R3, GNU 15, TCo319/O320, 8 MPI x 1 OMP, 6 h): WAMODEL
 * -10 %, SNONLIN -65 %, MPEXCHNG wait -55 %, all IFS norms bit-identical.
 *
 * x86 only; a no-op elsewhere.  Called from Fortran as
 *   CALL WAM_FTZ_ENTER(ISAVE)   /  CALL WAM_FTZ_LEAVE(ISAVE)
 */
#include <stdint.h>

#if defined(__x86_64__) || defined(__i386__)
#include <xmmintrin.h>
#define WAM_FTZ_DAZ_MASK 0x8040u   /* FTZ (bit 15) | DAZ (bit 6) */

void wam_ftz_enter_(int32_t *saved)
{
    unsigned int csr = _mm_getcsr();
    *saved = (int32_t)csr;
    _mm_setcsr(csr | WAM_FTZ_DAZ_MASK);
}

void wam_ftz_leave_(const int32_t *saved)
{
    _mm_setcsr((unsigned int)*saved);
}
#else
void wam_ftz_enter_(int32_t *saved) { *saved = 0; }
void wam_ftz_leave_(const int32_t *saved) { (void)saved; }
#endif
