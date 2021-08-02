/*
 *
 *    Copyright (c) 2014-2020
 *      SMASH Team
 *
 *    GNU General Public License (GPLv3 or later)
 *
 */

#ifndef SRC_INCLUDE_SMASH_POTENTIALS_H_
#define SRC_INCLUDE_SMASH_POTENTIALS_H_

#include <tuple>
#include <utility>
#include <vector>

#include "configuration.h"
#include "density.h"
#include "forwarddeclarations.h"
#include "particledata.h"
#include "threevector.h"

namespace smash {

/**
 * A class that stores parameters of potentials, calculates
 * potentials and their gradients. Potentials are responsible
 * for long-range interactions and stand in the left part of
 * Boltzmann equation. Short-range interactions are taken into
 * account in the right part of it - in the collision term.
 */
class Potentials {
 public:
  /**
   * Potentials constructor.
   *
   * \param[in] conf Configuration which contains the switches
   *            determining whether to turn on the Skyrme or the
   *            symmetry potentials, and the coefficents controlling
   *            how strong the potentials are.
   * \param[in] parameters Struct that contains the gaussian smearing factor
   *            \f$\sigma\f$, the distance cutoff \f$r_{\rm cut}\f$ and
   *            the testparticle number needed for the density calculation.
   */
  Potentials(Configuration conf, const DensityParameters &parameters);
  /// Standard destructor
  virtual ~Potentials();

  /**
   * Evaluates skyrme potential given a baryon density.
   *
   * \param[in] baryon_density Baryon density \f$\rho\f$ evaluated in the
   *            local rest frame in fm\f$^{-3}\f$.
   * \return Skyrme potential \f[U_B=10^{-3}\times\frac{\rho}{|\rho|}
   *         (A\frac{\rho}{\rho_0}+B(\frac{\rho}{\rho_0})^\tau)\f] in GeV
   */
  double skyrme_pot(const double baryon_density) const;

  /**
   * Evaluates symmetry potential given baryon isospin density.
   *
   * \note The second term is neglected if \f$\gamma\f$ is not specified in the
   * config. \param[in] baryon_isospin_density The difference between the proton
   * and the neutron density in the local rest frame in fm\f$^{-3}\f$.
   * \param[in] baryon_density
   * \return Symmetry potential \f[U_I=2\times 10^{-3}S_{\rm sym}
   *         \frac{\rho_{I_3}}{\rho_0}
   *         + \left[12.3\left(\frac{\rho_B}{\rho_0}\right)^{2/3}
   *         + 20\left(\frac{\rho_B}{\rho_0}\right)^\gamma\right]
   *         \left(\frac{\rho_{I_3}}{\rho_B}\right)^2\f] in GeV
   */
  double symmetry_pot(const double baryon_isospin_density,
                      const double baryon_density) const;

  /**
   * Calculate the factor \f$S(\rho)\f$ in the symmetry potential.
   *
   * \param[in] baryon_density baryon density
   * \return factor S in symmetry potenial
   */
  double symmetry_S(const double baryon_density) const;

  /**
   * Evaluates potential for the VDF model given the rest frame and the
   * computational frame baryon current.
   *
   * \param[in] nB rest frame baryon density, in fm\f$^{-3}\f$
   * \param[in] jmu_B_net net baryon current in the computational frame, in fm\f$^{-3}\f$
   * // Agnieszka correct
   * \return VDF potential \f[U_B=10^{-3}\times\frac{\rho}{|\rho|}
   *         (sum_i C_i (\frac{\rho}{\rho_0})^{b_i - 1})\f] in GeV
   */
  FourVector vdf_pot(double nB, const FourVector jmu_B_net) const;

  /**
   * Evaluates potential at point r. Potential is always taken in the local
   * Eckart rest frame, but point r is in the computational frame.
   *
   * \param[in] r Arbitrary space point where potential is calculated
   * \param[in] plist List of all particles to be used in \f$j^{\mu}\f$
   *            calculation. If the distance between particle and calculation
   *            point r, \f$ |r-r_i| > r_{cut} \f$ then particle input
   *            to density will be ignored.
   * \param[in] acts_on Type of particle on which potential is going to act.
   *            It gives the charges (or more precisely, the scaling factors)
   *		of the particle moving in the potential field.
   * \return Total potential energy acting on the particle: \f[U_{\rm tot}
   *         =Q_BU_B+2I_3U_I\f] in GeV, where \f$Q_B\f$ is the baryon charge
   *	     scaled by the ratio of the light (u, d) quark to the total quark
   *         number and \f$I_3\f$ is the third compnent of the isospin.
   */
  double potential(const ThreeVector &r, const ParticleList &plist,
                   const ParticleType &acts_on) const;

  /**
   * Evaluates the scaling factor of the forces acting on the particles.
   *
   * The forces are equal to the product of the scaling factor and the gradient
   * of the potential. We need these scaling factors to describe the motions of
   * the hyperons as well as the anti-particles in the potentials. For Lambda
   * and Sigma, since they carry 2 light (u or d) quarks, they are affected by
   * 2/3 of the Skyrme force. Xi carries 1 light quark, it is affected by 1/3 of
   * the Skyrme force. Omega carries no light quark, so it's not affected by the
   * Skyrme force. Anti-baryons are affected by the force as large as the force
   * acting on baryons but with an opposite direction.
   *
   * \param[in] data Type of particle on which potential is going to act.
   * \return (\f$Q_B(1-\frac{|Q_S|}{3}), Q_B\f$) where \f$Q_B\f$ is the baryon
   *         charge and \f$Q_S\f$ is the strangeness.
   */
  static std::pair<double, int> force_scale(const ParticleType &data);

  /**
   * Evaluates the electric and magnetic components of the skyrme force.
   *
   * \param[in] density Eckart baryon density [fm\f$^{-3}\f$].
   * \param[in] grad_rho Gradient of baryon density [fm\f$^{-4}\f$]. This
   * density is evaluated in the computational frame.
   * \param[in] dj_dt Time derivative of the baryon current density [fm\f$^{-4}\f$
   * \param[in] rot_j Curl of the baryon current density [fm\f$^{-4}\f$
   * \return (\f$E_B, B_B\f$), where \f[E_B = -V_B^\prime(\rho^\ast)(\nabla\rho_B
   *                                          + \partial_t \vec j_B)\f]
   *         is the electro component of Skyrme force and
   *         \f[B_B = V_B^\prime(\rho^\ast) \nabla\times\vec j_B\f]
   *         is the magnetic component of the Skyrme force
   *         with \f$\rho^\ast\f$ being the Eckart baryon density.
   */
  std::pair<ThreeVector, ThreeVector> skyrme_force(
      const double density, const ThreeVector grad_rho, const ThreeVector dj_dt,
      const ThreeVector rot_j) const;

  /**
   * Evaluates the electric and magnetic components of the symmetry force.
   *
   * \param[in] rhoI3 Relative isospin 3 density.
   * \param[in] grad_rhoI3 Gradient of I3/I density [fm\f$^{-4}\f$]. This
   *            density is evaluated in the computational frame.
   * \param[in] djI3_dt Time derivative of the I3/I current density
   *            [fm\f$^{-4}\f$]
   * \param[in] rot_jI3 Curl of the I3/I current density [fm\f$^{-4}\f$]
   * \param[in] rhoB Net-baryon density
   * \param[in] grad_rhoB Gradient of the net-baryon density
   * \param[in] djB_dt Time derivative of the net-baryon current density
   * \param[in] rot_jB Curl of the net-baryon current density
   * \return (\f$E_I3,
   *         B_I3\f$) [GeV/fm],
   *         where \f[\vec{E} = - \frac{\partial
   *         V^\ast}{\partial\rho_{I_3}^\ast}
   *         (\nabla\rho_{I_3} + \partial_t \vec j_{I_3})
   *         - \frac{\partial V^\ast}{\partial\rho_B^\ast}(\nabla\rho_B
   *         + \partial_t \vec j_B)\f]
   *         is the electrical component of symmetry force and
   *         \f[\vec{B} = \frac{\partial V^\ast}{\rho_{I_3}^\ast}
   *         \nabla\times\vec j_{I_3}
   *         + \frac{\partial V^\ast}{\rho_B^\ast}
   *         \nabla\times\vec j_B \f]
   *         is the magnetic component of the symmetry force
   *         with \f$\rho^\ast\f$ being the respective Eckart density.
   */
  std::pair<ThreeVector, ThreeVector> symmetry_force(
      const double rhoI3, const ThreeVector grad_rhoI3,
      const ThreeVector djI3_dt, const ThreeVector rot_jI3, const double rhoB,
      const ThreeVector grad_rhoB, const ThreeVector djB_dt,
      const ThreeVector rot_jB) const;

  /**
   * Evaluates the electric and magnetic components of the VDF force,
   * based on the VDF equations of motion.
   *
   * \param[in] nB rest frame baryon density in fm\f$^{-3}\f$
   * \param[in] dnB_dt time derivative of the rest frame density
   * \param[in] grad_nB gradient of the rest frame density
   * \param[in] gradnB_cross_jB cross product of gradnB and vec{j}
   * \param[in] j0 computational frame baryon density in fm\f$^{-3}\f$
   * \param[in] grad_j0 gradient of the computational frame current density
   * \param[in] vec_j baryon current
   * \param[in] dj_dt time derivative of the computational frema current density
   * \param[in] rot_j curl of the current density [fm\f$^{-4}\f$
   * \return [comment more on what it returns Agnieszka]
   *
   *
   * COMMENT
   */
  std::pair<ThreeVector, ThreeVector> vdf_force(
   double nB, const double dnB_dt,
   const ThreeVector grad_nB, const ThreeVector gradnB_cross_jB,
   const double j0, const ThreeVector grad_j0,
   const ThreeVector vec_j, const ThreeVector dj_dt, const ThreeVector rot_j) const;

  /**
   * Convenience overload of the above, using derivatives of the VDF mean-field
   * A^mu.
   * Evaluates the electric and magnetic components of the VDF force,
   * based on the VDF equations of motion.
   *
   * \param[in] grad_A_0 gradient of zeroth component of the field A^mu
   * \param[in] dA_dt time derivative of the field A^mu
   * \param[in] curl_vecA curl of the vector component of the field A^mu
   * \return [comment more on what it returns Agnieszka]
   *
   *
   * COMMENT
   */
 std::pair<ThreeVector, ThreeVector> v_df_force(
  const ThreeVector grad_A_0,
  const ThreeVector dA_dt,
  const ThreeVector curl_vecA) const;

  /**
   * Evaluates the electric and magnetic components of the forces at point r.
   * Point r is in the computational frame.
   *
   * \param[in] r Arbitrary space point where potential gradient is calculated
   * \param[in] plist List of all particles to be used in \f$j^{\mu}\f$
   *            calculation. If the distance between particle and calculation
   *            point r, \f$ |r-r_i| > r_{cut} \f$ then particle input
   *            to density will be ignored.
   * \return (\f$E_B, B_B, E_{I3}, B_{I3}\f$) [GeV/fm], where
   *          \f$E_B\f$: the electric component of the Skyrme force
   *          \f$B_B\f$: the magnetic component of the Skyrme force
   *          \f$E_{I3}\f$: the electric component of the symmetry force
   *          \f$B_{I3}\f$: the magnetic component of the symmetry force
   */
  virtual std::tuple<ThreeVector, ThreeVector, ThreeVector, ThreeVector>
  all_forces(const ThreeVector &r, const ParticleList &plist) const;

  /// \return Is Skyrme potential on?
  virtual bool use_skyrme() const { return use_skyrme_; }
  /// \return Is symmetry potential on?
  virtual bool use_symmetry() const { return use_symmetry_; }

  /// \return Skyrme parameter skyrme_a, in MeV
  double skyrme_a() const { return skyrme_a_; }
  /// \return Skyrme parameter skyrme_b, in MeV
  double skyrme_b() const { return skyrme_b_; }
  /// \return Skyrme parameter skyrme_tau
  double skyrme_tau() const { return skyrme_tau_; }
  /// \return Skyrme parameter S_pot, in MeV
  double symmetry_S_pot() const { return symmetry_S_Pot_; }


  /// \return Is VDF potential on?
  virtual bool use_vdf() const { return use_vdf_; }
  /// \return value of the saturation density used in the VDF potential
  virtual double saturation_density() const { return saturation_density_; }
  /// \return values of coefficients and powers in the VDF potential
  virtual double coeff_1() const { return coeff_1_; }
  virtual double coeff_2() const { return coeff_2_; }
  virtual double coeff_3() const { return coeff_3_; }
  virtual double coeff_4() const { return coeff_4_; }
  virtual double power_1() const { return power_1_; }
  virtual double power_2() const { return power_2_; }
  virtual double power_3() const { return power_3_; }
  virtual double power_4() const { return power_4_; }

 private:
  /**
   * Struct that contains the gaussian smearing width \f$\sigma\f$,
   * the distance cutoff \f$r_{\rm cut}\f$ and the testparticle number
   * needed for the density calculation.
   */
  const DensityParameters param_;

  /// Skyrme potential on/off
  bool use_skyrme_;

  /// Symmetry potential on/off
  bool use_symmetry_;

  /// VDF potential on/off
  bool use_vdf_;

  /**
   * Parameter of skyrme potentials:
   * the coefficient in front of \f$\frac{\rho}{\rho_0}\f$ in GeV
   */
  double skyrme_a_ = 0;

  /**
   * Parameters of skyrme potentials:
   * the coefficient in front of \f$(\frac{\rho}{\rho_0})^\tau\f$ in GeV
   */
  double skyrme_b_ = 0;

  /**
   * Parameters of skyrme potentials:
   * the power index.
   */
  double skyrme_tau_ = 0;

  /// Parameter S_Pot in the symmetry potential in MeV
  double symmetry_S_Pot_;

  /**
   * Whether the baryon density dependence of the symmetry potential is
   * included
   */
  bool symmetry_is_rhoB_dependent_ = false;
  /**
   * Power \f$ \gamma \f$ in formula for \f$ S(\rho) \f$:
   * \f[ S(\rho)=12.3\,\mathrm{MeV}\times
   * \left(\frac{\rho}{\rho_0}\right)^{2/3}+20\,\mathrm{MeV}\times
   * \left(\frac{\rho}{\rho_0}\right)^\gamma \f]
   */
  double symmetry_gamma_;



  /**
   * Saturation density of nuclear matter used in the VDF potential; it may
   * vary between different parameterizations.
   */
  double saturation_density_ = 0.0;
  /**
   * Parameters of VDF potential: the coefficients C_i and corresponding
   * powers of number density b_i. Default inclusion of 4 interaction terms.
   * Terms may be suppressed by putting C_i = 0, b_i = 0.
   */
  double coeff_1_ = 0.0;
  double coeff_2_ = 0.0;
  double coeff_3_ = 0.0;
  double coeff_4_ = 0.0;
  double power_1_ = 0.0;
  double power_2_ = 0.0;
  double power_3_ = 0.0;
  double power_4_ = 0.0;

  /**
   * Calculate the derivative of the symmetry potential with respect to
   * the isospin density in GeV * fm^3
   * \f[ \frac{\partial V_\mathrm{sym}}{\partial \rho_{I_3}}
   * = 2\frac{S_\mathrm{Pot}}{\rho_0}
   * + \frac{2\rho_{I_3}\left[12.3\left(\frac{\rho_B}{\rho_0}\right)^{2/3}
   * + 20 \left(\frac{\rho_B}{\rho_0}\right)^\gamma\right]}{\rho_B^2} \f]
   *
   * \note The isospin 3 density here is actually the density of I3 / I.
   *
   * \param[in] rhoB net baryon density
   * \param[in] rhoI3 isospin density
   * \return partial derivative of the symmetry potenital with respect to the
   * isospin density.
   */
  double dVsym_drhoI3(const double rhoB, const double rhoI3) const;

  /**
   * Calculate the derivative of the symmetry potential with respect to the
   * net baryon density in GeV * fm^3
   * \f[ \frac{\partial V_\mathrm{sym}}{\partial \rho_B} =
   * \left(\frac{\rho_{I_3}}{\rho_B}\right)^2
   * \left[\frac{8.2}{\rho_0}\left(\frac{\rho_B}{\rho_0}\right)^{-1/3}
   * + \frac{20\gamma}{\rho_B}\left(\frac{\rho_B}{\rho_0}\right)^\gamma\right]
   * -2\frac{\rho_{I_3}^2}{\rho_B^3}
   * \left[12.3\left(\frac{\rho_B}{\rho_0}\right)^{2/3}
   * + 20\left(\frac{\rho_B}{\rho_0}\right)^\gamma\right]\f]
   *
   * \note The isospin 3 density here is actually the density of I3 / I
   *
   * \param[in] rhoB net baryon density
   * \param[in] rhoI3 isospin density
   * \return partial derivative of the symmetry potenital with respect to the
   *         net baryon density.
   */
  double dVsym_drhoB(const double rhoB, const double rhoI3) const;
};

}  // namespace smash

#endif  // SRC_INCLUDE_SMASH_POTENTIALS_H_
