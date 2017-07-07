/*! \page page_logging Logging
 * 
 * 
 * \section logging_in_c Logging in C
 * In SuperSCS, we may activate the logging of progress information using 
 * the property <code>do_record_progress</code> in \ref SCS_SETTINGS "Settings".
 * 
 * In that case, the \ref SCS_INFO "Info" object that is updated using ::scs contains the 
 * following information:
 * 
 * - <code>progress_iter</code> an array of the iterations when the progress data
 *   were recorded. It is possible that progress data are stored with a stride.
 * - <code>progress_time</code> an array of timestamps (starting at \c 0) when
 *   the progress data were recorded
 * - <code>progress_norm_fpr</code> the values of the fixed-point residual at 
 *   every iteration.
 * - <code>progress_respri</code> the recoded primal residuals
 * - <code>progress_resdual</code> the recorded dual residuals
 * - <code>progress_relgap</code> an array of the recoded values of the relative
 *   gap
 * - <code>progress_pcost</code> an array of the recoded values of the primal cost
 * - <code>progress_dcost</code> an array of the recoded values of the dual cost 
 * - <code>progress_ls</code> the number of line-search iterations at each iteration
 *   of the algorithm
 * - <code>progress_mode</code> the type of step that was taken at every iteration;
 *   \c -1 corresponds to a nominal step, \c 0 is for a K0 step and \c 1 and \c 2
 *   are for K1 and K2 steps respectively.
 * 
 * Note that the above arrays have length equal to the number of iterations.
 * 
 * Example:
 * 
 * ~~~~~{.m}
 * scs_int i;
 * const scs_int column_size = 10;
 * printf("  i     P Cost    D Cost       Gap       FPR      PRes      DRes\n");
 * printf("----------------------------------------------------------------\n");
 * for (i = 0; i < info->iter; ++i) {
 *       printf("%*i ", 3, info->progress_iter[i]);
 *       printf("%*.2e", column_size, info->progress_pcost[i]);
 *       printf("%*.2e", column_size, info->progress_dcost[i]);
 *       printf("%*.2e", column_size, info->progress_relgap[i]);
 *       printf("%*.2e", column_size, info->progress_norm_fpr[i]);
 *       printf("%*.2e", column_size, info->progress_respri[i]);
 *       printf("%*.2e", column_size, info->progress_resdual[i]);
 *       printf("\n");
 * }
 * ~~~~~
 * 
 * 
 * 
 * \section logging_in_matlab Logging in MATLAB
 * Similarly, we may record progress information in MATLAB when we call 
 * <code>scs_direct.m</code>, <code>scs_indirect.m</code> or <code>scs.m</code>.
 * 
 * All we have to do is to set <code>do_record_progress</code> to \c 1.
 * 
 * ~~~~~{.m}
 * params.do_record_progress = 1; 
 * [x, y, s, info] = scs_direct(data, K, params);
 * ~~~~~
 * 
 * This is the \c info structure we get when we run 
 * [tests/matlab/cone_test.m](https://github.com/kul-forbes/scs/blob/master/tests/matlab/cone_test.m):
 * 
 * ~~~~~{.m}
 * info1 = 
 * 
 *                       iter: 32
 *                     status: 'Solved'
 *                       pobj: -16.3754
 *                       dobj: -16.3754
 *                     resPri: 6.7545e-12
 *                    resDual: 2.2441e-11
 *                  resInfeas: NaN
 *                   resUnbdd: 0.3220
 *                     relGap: 2.6977e-12
 *                  setupTime: 0.0507
 *                  solveTime: 1.2677
 *              progress_iter: [32x1 double]
 *            progress_relgap: [32x1 double]
 *            progress_respri: [32x1 double]
 *           progress_resdual: [32x1 double]
 *             progress_pcost: [32x1 double]
 *             progress_dcost: [32x1 double]
 *              progress_time: [32x1 double]
 *              progress_mode: [32x1 double]
 *                progress_ls: [32x1 double]
 * ~~~~~
 */