/* Author:  G. Jungman
 * RCS:     $Id$
 */
#include <config.h>
#include <gsl_math.h>
#include <gsl_errno.h>
#include "gsl_sf_gamma.h"
#include "gsl_sf_coupling.h"


#ifdef HAVE_INLINE
inline
#endif
static
int locMax3(const int a, const int b, const int c)
{
  int d = GSL_MAX(a, b);
  return GSL_MAX(d, c);
}

#ifdef HAVE_INLINE
inline
#endif
static
int locMin3(const int a, const int b, const int c)
{
  int d = GSL_MIN(a, b);
  return GSL_MIN(d, c);
}

#ifdef HAVE_INLINE
inline
#endif
static
int locMin5(const int a, const int b, const int c, const int d, const int e)
{
  int f = GSL_MIN(a, b);
  int g = GSL_MIN(c, d);
  int h = GSL_MIN(f, g);
  return GSL_MIN(e, h);
}


/* See: [Thompson, Atlas for Computing Mathematical Functions] */

static
int
delta(int ta, int tb, int tc, gsl_sf_result * d)
{
  gsl_sf_result f1, f2, f3, f4;
  int status = 0;
  status += gsl_sf_fact_impl((ta + tb - tc)/2, &f1);
  status += gsl_sf_fact_impl((ta + tc - tb)/2, &f2);
  status += gsl_sf_fact_impl((tb + tc - ta)/2, &f3);
  status += gsl_sf_fact_impl((ta + tb + tc)/2 + 1, &f4);
  if(status != 0) {
    d->val = 0.0;
    d->err = 0.0;
    return GSL_EOVRFLW;
  }
  d->val = f1.val * f2.val * f3.val / f4.val;
  d->err = 4.0 * GSL_DBL_EPSILON * fabs(d->val);
  return GSL_SUCCESS;
}


static
int
triangle_selection_fails(int two_ja, int two_jb, int two_jc)
{
  return ((two_jb < abs(two_ja - two_jc)) || (two_jb > two_ja + two_jc));
}


static
int
m_selection_fails(int two_ja, int two_jb, int two_jc,
                  int two_ma, int two_mb, int two_mc)
{
  return (   abs(two_ma) > two_ja 
          || abs(two_mb) > two_jb
	  || abs(two_mc) > two_jc
	  || GSL_IS_ODD(two_ja + two_ma)
	  || GSL_IS_ODD(two_jb + two_mb)
	  || GSL_IS_ODD(two_jc + two_mc)
	  );
}


/*-*-*-*-*-*-*-*-*-*-*-* (semi)Private Implementations *-*-*-*-*-*-*-*-*-*-*-*/

int
gsl_sf_coupling_3j_impl(int two_ja, int two_jb, int two_jc,
                        int two_ma, int two_mb, int two_mc,
			gsl_sf_result * result)
{
  if(result == 0) {
    return GSL_EFAULT;
  }
  else if(two_ja < 0 || two_jb < 0 || two_jc < 0) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_EDOM;
  }
  else if(   triangle_selection_fails(two_ja, two_jb, two_jc)
          || m_selection_fails(two_ja, two_jb, two_jc, two_ma, two_mb, two_mc)
     ) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_SUCCESS;
  }
  else {
    gsl_sf_result n1_a, n1_b, n3_a, n3_b;
    gsl_sf_result d1_a, d1_b, d2_a, d2_b, d3_a, d3_b;
    gsl_sf_result n1, n2, n3;
    gsl_sf_result d1, d2, d3;
    double norm;
    double sign = (GSL_IS_ODD((two_ja - two_jb - two_mc)/2) ? -1.0 : 1.0);
    int tk, tkmin, tkmax;
    double sum_pos = 0.0;
    double sum_neg = 0.0;
    double phase;
    int status = 0;
    status += gsl_sf_fact_impl((two_jc + two_ja - two_jb)/2, &n1_a);
    status += gsl_sf_fact_impl((two_jc - two_ja + two_jb)/2, &n1_b);
    status += gsl_sf_fact_impl((two_ja + two_jb - two_jc)/2, &n2);
    status += gsl_sf_fact_impl((two_jc - two_mc)/2, &n3_a);
    status += gsl_sf_fact_impl((two_jc + two_mc)/2, &n3_b);
    status += gsl_sf_fact_impl((two_ja + two_jb + two_jc)/2 + 1, &d1);
    status += gsl_sf_fact_impl((two_ja - two_ma)/2, &d2_a);
    status += gsl_sf_fact_impl((two_ja + two_ma)/2, &d2_b);
    status += gsl_sf_fact_impl((two_jb - two_mb)/2, &d3_a);
    status += gsl_sf_fact_impl((two_jb + two_mb)/2, &d3_b);

    if(status != 0) {
      result->val = 0.0;
      result->err = 0.0;
      return GSL_EOVRFLW;
    }

    n1.val = n1_a.val * n1_b.val;
    n3.val = n3_a.val * n3_b.val;
    d2.val = d2_a.val * d2_b.val;
    d3.val = d3_a.val * d3_b.val;

    norm = sign * sqrt(n1.val*n2.val*n3.val)/sqrt(d1.val*d2.val*d3.val);

    tkmin = GSL_MAX(0, two_jb - two_ja - two_mc);
    tkmax = GSL_MIN(two_jc - two_ja + two_jb, two_jc - two_mc);
    
    phase = GSL_IS_ODD((tkmin + two_jb + two_mb)/2) ? -1.0 : 1.0;

    for(tk=tkmin; tk<=tkmax; tk += 2) {
      double term;

      status = 0;
      status += gsl_sf_fact_impl((two_jb + two_jc + two_ma - tk)/2, &n1);
      status += gsl_sf_fact_impl((two_ja - two_ma + tk)/2, &n2);
      status += gsl_sf_fact_impl(tk/2, &d1_a);
      status += gsl_sf_fact_impl((two_jc - two_ja + two_jb - tk)/2, &d1_b);
      status += gsl_sf_fact_impl((two_jc - two_mc - tk)/2, &d2);
      status += gsl_sf_fact_impl((two_ja - two_jb + two_mc + tk)/2, &d3);

      if(status != 0) {
        result->val = 0.0;
	result->err = 0.0;
	return GSL_EOVRFLW;
      }

      d1.val = d1_a.val * d1_b.val;

      term = phase * n1.val * n2.val / (d1.val * d2.val * d3.val);
      phase = -phase;

      if(norm*term >= 0.0) {
        sum_pos += norm*term;
      }
      else {
        sum_neg -= norm*term;
      }
    }

    result->val  = sum_pos - sum_neg;
    result->err  = 2.0 * GSL_DBL_EPSILON * (sum_pos + sum_neg);
    result->err += 2.0 * GSL_DBL_EPSILON * (tkmax - tkmin) * fabs(result->val);

    return GSL_SUCCESS;
  }
}


int
gsl_sf_coupling_6j_impl(int two_ja, int two_jb, int two_jc,
                        int two_jd, int two_je, int two_jf,
			gsl_sf_result * result)
{
  if(result == 0) {
    return GSL_EFAULT;
  }
  else if(   two_ja < 0 || two_jb < 0 || two_jc < 0
     || two_jd < 0 || two_je < 0 || two_je < 0
     ) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_EDOM;
  }
  else if(   triangle_selection_fails(two_ja, two_jb, two_je)
          || triangle_selection_fails(two_ja, two_jc, two_jf)
          || triangle_selection_fails(two_jb, two_jd, two_jf)
          || triangle_selection_fails(two_jc, two_jd, two_je)
     ) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_SUCCESS;
  }
  else {
    gsl_sf_result n1;
    gsl_sf_result d1, d2, d3, d4, d5, d6;
    double norm;
    int tk, tkmin, tkmax;
    double phase;
    double sum_pos = 0.0;
    double sum_neg = 0.0;
    double sumsq_err = 0.0;
    int status = 0;
    status += delta(two_ja, two_jb, two_je, &d1);
    status += delta(two_ja, two_jc, two_jf, &d2);
    status += delta(two_jb, two_jd, two_jf, &d3);
    status += delta(two_jc, two_jd, two_je, &d4);
    if(status != GSL_SUCCESS) {
      result->val = 0.0;
      result->err = 0.0;
      return GSL_EOVRFLW;
    }
    norm = sqrt(d1.val) * sqrt(d2.val) * sqrt(d3.val) * sqrt(d4.val);
    
    tkmin = locMax3(0,
                   two_ja + two_jd - two_je - two_jf,
                   two_jb + two_jc - two_je - two_jf);

    tkmax = locMin5(two_ja + two_jb + two_jc + two_jd + 2,
                    two_ja + two_jb - two_je,
		    two_jc + two_jd - two_je,
		    two_ja + two_jc - two_jf,
		    two_jb + two_jd - two_jf);

    phase = GSL_IS_ODD((two_ja + two_jb + two_jc + two_jd + tkmin)/2)
            ? -1.0
	    :  1.0;

    for(tk=tkmin; tk<=tkmax; tk += 2) {
      double term;
      double term_err;
      gsl_sf_result den_1, den_2;
      gsl_sf_result d1_a, d1_b;
      status = 0;
      
      status += gsl_sf_fact_impl((two_ja + two_jb + two_jc + two_jd - tk)/2 + 1, &n1);
      status += gsl_sf_fact_impl(tk/2, &d1_a);
      status += gsl_sf_fact_impl((two_je + two_jf - two_ja - two_jd + tk)/2, &d1_b);
      status += gsl_sf_fact_impl((two_je + two_jf - two_jb - two_jc + tk)/2, &d2);
      status += gsl_sf_fact_impl((two_ja + two_jb - two_je - tk)/2, &d3);
      status += gsl_sf_fact_impl((two_jc + two_jd - two_je - tk)/2, &d4);
      status += gsl_sf_fact_impl((two_ja + two_jc - two_jf - tk)/2, &d5);
      status += gsl_sf_fact_impl((two_jb + two_jd - two_jf - tk)/2, &d6);
      
      if(status != GSL_SUCCESS) {
        result->val = 0.0;
	result->err = 0.0;
	return GSL_EOVRFLW;
      }

      d1.val = d1_a.val * d1_b.val;
      d1.err = d1_a.err * fabs(d1_b.val) + fabs(d1_a.val) * d1_b.err;

      den_1.val  = d1.val*d2.val*d3.val;
      den_1.err  = d1.err * fabs(d2.val*d3.val);
      den_1.err += d2.err * fabs(d1.val*d3.val);
      den_1.err += d3.err * fabs(d1.val*d2.val);

      den_2.val  = d4.val*d5.val*d6.val;
      den_2.err  = d4.err * fabs(d5.val*d6.val);
      den_2.err += d5.err * fabs(d4.val*d6.val);
      den_2.err += d6.err * fabs(d4.val*d5.val);

      term  = phase * n1.val / den_1.val / den_2.val;
      phase = -phase;
      term_err  = n1.err / fabs(den_1.val) / fabs(den_2.val);
      term_err += fabs(term / den_1.val) * den_1.err;
      term_err += fabs(term / den_2.val) * den_2.err;

      if(term >= 0.0) {
        sum_pos += norm*term;
      }
      else {
        sum_neg -= norm*term;
      }

      sumsq_err += norm*norm * term_err*term_err;
    }

    result->val  = sum_pos - sum_neg;
    result->err  = 2.0 * GSL_DBL_EPSILON * (sum_pos + sum_neg);
    result->err += sqrt(sumsq_err / (0.5*(tkmax-tkmin)+1.0));
    result->err += 2.0 * GSL_DBL_EPSILON * (tkmax - tkmin + 2.0) * fabs(result->val);

    return GSL_SUCCESS;
  }
}


int
gsl_sf_coupling_9j_impl(int two_ja, int two_jb, int two_jc,
                        int two_jd, int two_je, int two_jf,
			int two_jg, int two_jh, int two_ji,
			gsl_sf_result * result)
{
  if(result == 0) {
    return GSL_EFAULT;
  }
  else if(   two_ja < 0 || two_jb < 0 || two_jc < 0
     || two_jd < 0 || two_je < 0 || two_jf < 0
     || two_jg < 0 || two_jh < 0 || two_ji < 0
     ) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_EDOM;
  }
  else if(   triangle_selection_fails(two_ja, two_jb, two_jc)
          || triangle_selection_fails(two_jd, two_je, two_jf)
          || triangle_selection_fails(two_jg, two_jh, two_ji)
          || triangle_selection_fails(two_ja, two_jd, two_jg)
          || triangle_selection_fails(two_jb, two_je, two_jh)
          || triangle_selection_fails(two_jc, two_jf, two_ji)
     ) {
    result->val = 0.0;
    result->err = 0.0;
    return GSL_SUCCESS;
  }
  else {
    int tk;
    int tkmin = locMax3(abs(two_ja-two_ji), abs(two_jh-two_jd), abs(two_jb-two_jf));
    int tkmax = locMin3(two_ja + two_ji, two_jh + two_jd, two_jb + two_jf);
    double sum_pos = 0.0;
    double sum_neg = 0.0;
    double sumsq_err = 0.0;
    double phase;
    for(tk=tkmin; tk<=tkmax; tk += 2) {
      gsl_sf_result s1, s2, s3;
      double term;
      double term_err;
      int status = 0;
      status += gsl_sf_coupling_6j_impl(two_ja, two_ji, two_jd,  two_jh, tk, two_jg,  &s1);
      status += gsl_sf_coupling_6j_impl(two_jb, two_jf, two_jh,  two_jd, tk, two_je,  &s2);
      status += gsl_sf_coupling_6j_impl(two_ja, two_ji, two_jb,  two_jf, tk, two_jc,  &s3);
      if(status != GSL_SUCCESS) {
        result->val = 0.0;
	result->err = 0.0;
	return GSL_EOVRFLW;
      }
      term = s1.val * s2.val * s3.val;
      term_err  = s1.err * fabs(s2.val*s3.val);
      term_err += s2.err * fabs(s1.val*s3.val);
      term_err += s3.err * fabs(s1.val*s2.val);

      if(term >= 0.0) {
        sum_pos += (tk + 1) * term;
      }
      else {
        sum_neg -= (tk + 1) * term;
      }

      sumsq_err += ((tk+1) * term_err) * ((tk+1) * term_err);
    }

    phase = GSL_IS_ODD(tkmin) ? -1.0 : 1.0;

    result->val  = phase * (sum_pos - sum_neg);
    result->err  = 2.0 * GSL_DBL_EPSILON * (sum_pos + sum_neg);
    result->err += sqrt(sumsq_err / (0.5*(tkmax-tkmin)+1.0));
    result->err += 2.0 * GSL_DBL_EPSILON * (tkmax-tkmin + 2.0) * fabs(result->val);

    return GSL_SUCCESS;
  }
}


/*-*-*-*-*-*-*-*-*-*-*-* Functions w/ Error Handling *-*-*-*-*-*-*-*-*-*-*-*/

int gsl_sf_coupling_3j_e(int two_ja, int two_jb, int two_jc,
                         int two_ma, int two_mb, int two_mc,
		         gsl_sf_result * result)
{
  int status = gsl_sf_coupling_3j_impl(two_ja, two_jb, two_jc,
                                       two_ma, two_mb, two_mc,
				       result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_coupling_3j_e", status);
  }
  return status;
}


int gsl_sf_coupling_6j_e(int two_ja, int two_jb, int two_jc,
                         int two_jd, int two_je, int two_jf,
			 gsl_sf_result * result)
{
  int status = gsl_sf_coupling_6j_impl(two_ja, two_jb, two_jc,
                                       two_jd, two_je, two_jf,
				       result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_coupling_6j_e", status);
  }
  return status;
}


int gsl_sf_coupling_9j_e(int two_ja, int two_jb, int two_jc,
                         int two_jd, int two_je, int two_jf,
			 int two_jg, int two_jh, int two_ji,
			 gsl_sf_result * result)
{
  int status = gsl_sf_coupling_9j_impl(two_ja, two_jb, two_jc,
                                       two_jd, two_je, two_jf,
				       two_jg, two_jh, two_ji,
				       result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_coupling_9j_e", status);
  }
  return status;
}
