// [[Rcpp::depends(RcppEigen)]]
#include <vector>
#include <cmath>
#include <Rcpp.h>
#include <RcppEigen.h>

double sigmoid(double x) {
  return 1.0 / (1.0 + std::exp(-x));
}

void preds_calc(Eigen::MatrixXd& res,
                const Eigen::MatrixXd& beta_lm,
                const Eigen::MatrixXd& beta_glm,
                const Eigen::MatrixXd& dmat_lm,
                const Eigen::MatrixXd& dmat_glm,
                const Eigen::MatrixXd& u_lm,
                const Eigen::MatrixXd& u_glm,
                int j,
                int B) {
  
  // these are N_j x B
  Eigen::MatrixXd pred_lm_j = (dmat_lm * beta_lm.transpose());
  Eigen::MatrixXd pred_glm_j = (dmat_glm * beta_glm.transpose());
  
  pred_lm_j.rowwise() += u_lm.col(j).transpose();
  pred_glm_j.rowwise() += u_glm.col(j).transpose();
  
  pred_glm_j = pred_glm_j.unaryExpr(&sigmoid);
  Eigen::MatrixXd unit_preds_j = pred_lm_j.cwiseProduct(pred_glm_j);
  
  Eigen::MatrixXd dom_preds_j = unit_preds_j.colwise().mean();
  res.block(j, 0, 1, B) = dom_preds_j;
  
}

//[[Rcpp::export]]
SEXP generate_preds(const Eigen::MatrixXd& beta_lm,
                    const Eigen::MatrixXd& beta_glm,
                    const Eigen::MatrixXd& u_lm,
                    const Eigen::MatrixXd& u_glm,
                    const Rcpp::List& design_mats,
                    int J) {
  
  int B = u_lm.rows();
  
  // initialize result matrix
  Eigen::MatrixXd result = Eigen::MatrixXd::Zero(J, B);
  
  for (int j = 0; j < J; ++j) {
    
    Rcpp::List dmats_j = design_mats[j];
    Eigen::MatrixXd dmat_lm_j = dmats_j[0];
    Eigen::MatrixXd dmat_glm_j = dmats_j[1];
    
    preds_calc(result,
               beta_lm,
               beta_glm,
               dmat_lm_j,
               dmat_glm_j,
               u_lm,
               u_glm,
               j, 
               B);
    
  }
  
  return Rcpp::wrap(result);
  
}