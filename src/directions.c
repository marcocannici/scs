#include "directions.h"


/*TODO Allocate this variable within work */
static scs_float * HY; /* Vector H*Y of length l */

scs_int resetDirectionCache(DirectionCache * cache) {
    cache->mem_cursor = 0; /* set active memory to 0 */
    cache->current_mem = 0;
    RETURN DIRECTION_CACHE_RESET;
}

scs_int computeAndersonDirection(Work *work) {
    /* --- DECLARATIONS --- */
    DirectionCache * cache; /* the SU cache (pointer) */
    const scs_int l = work->l; /* size of vectors */
    scs_float * s_current;
    scs_float * y_current;
    scs_float * s_minus_y_current;
    scs_int colsY;

#ifdef SVD_ACTIVATED
    scs_float rcond = 1e-8;
    scs_int rank;
    scs_float * singular_values;
#endif
    
    scs_float * copy_cache_Y;
    
    cache = work->direction_cache;

#ifdef SVD_ACTIVATED    
    singular_values = cache->ls_wspace + cache->ls_wspace_length;
    copy_cache_Y = singular_values + cache->mem;
#else
    copy_cache_Y = cache->ls_wspace + cache->ls_wspace_length;
#endif



    /* d [work->dir] = -R [work->R] */
    setAsScaledArray(work->dir, work->R, -1.0, l);

    s_current = cache->S + (cache->mem_cursor * l);
    y_current = cache->U + (cache->mem_cursor * l);
    s_minus_y_current = cache->S_minus_Y + (cache->mem_cursor * l);

    memcpy(s_current, work->Sk, l * sizeof (scs_float));
    memcpy(y_current, work->Yk, l * sizeof (scs_float));

    cache->current_mem++; /* increment the current memory */
    if (cache->current_mem > cache->mem)
        cache->current_mem = cache->mem;

    colsY = cache->current_mem;

    /* Construct matrix S-Y by subtracting one pair of vecs at a time */
    axpy2(s_minus_y_current, s_current, y_current, 1.0, -1.0, l);

    /* Solve Yt = R with SVD (we need to copy Y into a different memory positions
     * because svdls modifies it (see the documentation of svdls). */
    memcpy(cache->t, work->R, l * sizeof (scs_float));
    memcpy(copy_cache_Y, cache->U, l * colsY * sizeof (scs_float));

#ifdef SVD_ACTIVATED
    /* Solve LS with SVD */
    svdls(l, colsY,
            copy_cache_Y,
            cache->t,
            cache->ls_wspace,
            cache->ls_wspace_length,
            rcond,
            singular_values,
            &rank);
#else   
    /* Solve LS with QR */
    qrls(l, colsY,
            copy_cache_Y,
            cache->t,
            cache->ls_wspace,
            cache->ls_wspace_length);
#endif

    /* dir = dir - S_minus_Y * t */
    matrixMultiplicationColumnPacked(l, 1, colsY, -1.0,
            cache->S_minus_Y, 1.0, cache->t, work->dir);

    cache->mem_cursor++; /* move the cursor */
    if (cache->mem_cursor >= cache->mem)
        cache->mem_cursor = 0;

    return DIRECTION_SUCCESS;
}

scs_int computeLSBroyden(Work *work) {
    /* --- DECLARATIONS --- */
    DirectionCache * cache; /* the SU cache (pointer) */
    scs_int i; /* index */
    scs_float * s_tilde_current; /* s_tilde (which is updated) */
    scs_float * u_new; /* new value of u */
    scs_float ip; /* temporary float to store inner products */
    scs_float s_norm_sq; /* scalar gamma as in (6.5e) */
    scs_float theta = 0; /* theta */
    const scs_int l = work->l; /* size of vectors */
    const scs_float theta_bar = work->stgs->thetabar; /* parameter in Powell's trick */

    cache = work->direction_cache; /* cache of Sk and Uk */

    /* d [work->dir] = -R [work->R] */
    setAsScaledArray(work->dir, work->R, -1.0, l);

    /* s_tilde_current = y [work->Yk]                                           */
    /* use the end of the cache to store s_tilde_current                        */
    /* later we use the same position of the S-buffer to store the current Sk   */
    s_tilde_current = cache->S + (cache->mem_cursor * l);
    memcpy(s_tilde_current, work->Yk, l * sizeof (scs_float));

    /* update s and d */
    for (i = 0; i < cache->mem_cursor; ++i) {
        scs_float * s_i; /* pointer to the current value of s_i */
        scs_float * u_i; /* pointer to the current value of u_i */
        s_i = cache->S + i * l; /* retrieve s_i from the cache */
        u_i = cache->U + i * l; /* retrieve u_i from the cache */
        ip = innerProd(s_i, s_tilde_current, l);
        addScaledArray(s_tilde_current, u_i, l, ip); /* update s_tilde */
        ip = innerProd(s_i, work->dir, l);
        addScaledArray(work->dir, u_i, l, ip); /* update direction */
    }

    /* compute theta */
    ip = innerProd(s_tilde_current, work->Sk, l);
    s_norm_sq = calcNormSq(work->Sk, l);

    if (ABS(ip) >= theta_bar * s_norm_sq) {
        theta = 1;
    } else {
        theta = s_norm_sq * (1 - SGN(ip) * theta_bar) / (s_norm_sq - ip);
        /* s_tilde_current = (1-theta)*s + theta*s_tilde_current */
        axpy2(s_tilde_current, s_tilde_current, work->Sk, theta, 1 - theta, l);
    }

    /* FINALISE */

    /* update u_new (at the end of the buffer) */
    u_new = cache->U + (cache->mem_cursor * l);
    ip = (1 - theta) * s_norm_sq + theta * ip;
    for (i = 0; i < l; ++i) {
        u_new[i] = (work->Sk[i] - s_tilde_current[i]) / ip;
    }
    /* update direction */
    ip = innerProd(work->Sk, work->dir, l); /* s'd */
    addScaledArray(work->dir, u_new, l, ip);

    /* push s into the buffer */
    memcpy(s_tilde_current, work->Sk, l * sizeof (scs_float));

    cache->mem_cursor++; /* move the cursor */

    /* if the cursor has exceeded the last position, reset the cache */
    if (cache->mem_cursor >= cache->mem) {
        RETURN resetDirectionCache(cache); /* returns DIRECTION_CACHE_RESET */
    }

    RETURN DIRECTION_CACHE_INCREMENT;
}

/* LCOV_EXCL_START */
scs_int computeFullBroyden(Work *work, scs_int i) {
    scs_float ip = 0;
    scs_float tmp = 0;

    if (i == 0 || HY == SCS_NULL) {
        /* HY is allocated the first time this function is called (that is, for i==0) */
        HY = malloc(work->l * sizeof (*HY));
        if (HY == SCS_NULL) {
            scs_printf("ERROR: allocating `HY` in `computeFullBroyden` failure\n");
            RETURN DIRECTION_ERROR;
        }
    }

    if ((work->stgs->broyden_init_scaling && i == 1)
            || (work->stgs->tRule == 1 || work->stgs->tRule == 2)) {
        ip = innerProd(work->Yk, work->Sk, work->l);
    }

    if (work->stgs->broyden_init_scaling && i == 1) {
        scs_int i;
        tmp = ip / calcNorm(work->Yk, work->l);
        for (i = 0; i < work->l; ++i) {
            work->H[i * (work->l + 1)] = tmp;
        }
    }

    return DIRECTION_SUCCESS;
}

/* LCOV_EXCL_STOP */

scs_int computeDirection(Work *work, scs_int i) {
    scs_int status = DIRECTION_SUCCESS;

    switch (work->stgs->direction) {
        case fixed_point_residual:
            setAsScaledArray(work->dir, work->R, -1.0, work->l); /* dir = -R */
            status = DIRECTION_SUCCESS;
            break;
        case restarted_broyden:
            status = computeLSBroyden(work);
            break;
        case anderson_acceleration:
            status = computeAndersonDirection(work);
            break;
        case full_broyden:
            status = computeFullBroyden(work, i);
            break;
        default:
            /* Not implemented yet */
            status = DIRECTION_ERROR;
    }

    RETURN status;
}

/* LCOV_EXCL_START */
void freeFullBroyden() {
    if (HY != SCS_NULL) {
        scs_free(HY);
    }
}
/* LCOV_EXCL_STOP */
