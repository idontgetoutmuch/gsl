/* Author:  G. Jungman
 * RCS:     $Id$
 */
#ifndef __GSL_SF_PSI_H__
#define __GSL_SF_PSI_H__

#include <gsl/gsl_sf_result.h>


/* Poly-Gamma Functions
 *
 * psi(m,x) := (d/dx)^m psi(0,x) = (d/dx)^{m+1} log(gamma(x))
 */


/* Di-Gamma Function  psi(n)
 *
 * n > 0
 * exceptions: GSL_EDOM
 */
int     gsl_sf_psi_int_impl(int n, gsl_sf_result * result);
int     gsl_sf_psi_int_e(int n, gsl_sf_result * result);


/* Di-Gamma Function psi(x)
 *
 * x != 0.0
 * exceptions: GSL_EDOM, GSL_ELOSS
 */
int     gsl_sf_psi_impl(double x, gsl_sf_result * result);
int     gsl_sf_psi_e(double x, gsl_sf_result * result);


/* Di-Gamma Function Re[psi(1 + I y)]
 *
 * exceptions: none
 */
int     gsl_sf_psi_1piy_impl(double y, gsl_sf_result * result);
int     gsl_sf_psi_1piy_e(double y, gsl_sf_result * result);


/* Tri-Gamma Function psi^(1)(n)
 *
 * n > 0
 * exceptions: GSL_EDOM
 */
int     gsl_sf_psi_1_int_impl(int n, gsl_sf_result * result);
int     gsl_sf_psi_1_int_e(int n, gsl_sf_result * result);


/* Poly-Gamma Function psi^(n)(x)
 *
 * n >= 0, x > 0.0
 * exceptions: GSL_EDOM
 */
int     gsl_sf_psi_n_impl(int n, double x, gsl_sf_result * result);
int     gsl_sf_psi_n_e(int n, double x, gsl_sf_result * result);


#endif /* __GSL_SF_PSI_H__ */
