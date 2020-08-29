/*
 *******************************************************************************************
 * Author	: Sang Hyun Son
 * Email	: shh1295@gmail.com
 * Github	: github.com/SonSang
 *******************************************************************************************
 */

#include "TorusApprox.h"
#include "../Circle/Circle.h"

namespace MN {
	// TorusApprox Mapping
	void TorusApprox::Mapping::set(const SurfaceInfo& surface, const TorusApprox& torus, Real m, Real n) {
		Real u = surface.u;
		Real v = surface.v;
		m0 = m;
		n0 = n;

		Vec3 Fuu, Fuv, Fvv, Gmm, Gmn, Gnn, Fu, Fv, Gm, Gn;
		Fuu = surface.Fuu;
		Fuv = surface.Fuv;
		Fvv = surface.Fvv;
		Fu = surface.Fu;
		Fv = surface.Fv;

		Gmm = torus.transform.applyR(torus.patch.differentiate(m, n, 2, 0));
		Gmn = torus.transform.applyR(torus.patch.differentiate(m, n, 1, 1));
		Gnn = torus.transform.applyR(torus.patch.differentiate(m, n, 0, 2));
		Gm = torus.transform.applyR(torus.patch.differentiate(m, n, 1, 0));
		Gn = torus.transform.applyR(torus.patch.differentiate(m, n, 0, 1));

		// M, N Coefs
		{
			Real
				Gm_len = Gm.len(),
				Gn_len = Gn.len(),
				Gm_len2 = Gm_len * Gm_len,
				Gn_len2 = Gn_len * Gn_len;
			Real
				A = Vec3::factorize(Fu, Gm, true) / Gm_len2,
				B = Vec3::factorize(Fu, Gn, true) / Gn_len2,
				C = Vec3::factorize(Fv, Gm, true) / Gm_len2,
				D = Vec3::factorize(Fv, Gn, true) / Gn_len2;
			Vec3 helperA, helperB, helperC;
			helperA = Fuu - Gmm * A * A - Gmn * 2 * A * B - Gnn * B * B;
			helperB = Fvv - Gmm * C * C - Gmn * 2 * C * D - Gnn * D * D;
			helperC = Fuv - Gmm * A * C - Gmn * (A * D + B * C) - Gnn * C * D;

			mCoefs[0] = Vec3::factorize(helperA, Gm, true) * 0.5 / Gm_len2;
			mCoefs[1] = Vec3::factorize(helperB, Gm, true) * 0.5 / Gm_len2;
			mCoefs[2] = Vec3::factorize(helperC, Gm, true) / Gm_len2;
			mCoefs[3] = A - 2 * mCoefs[0] * u - mCoefs[2] * v;
			mCoefs[4] = C - 2 * mCoefs[1] * v - mCoefs[2] * u;
			mCoefs[5] = m - mCoefs[0] * u * u - mCoefs[1] * v * v - mCoefs[2] * u * v - mCoefs[3] * u - mCoefs[4] * v;

			nCoefs[0] = Vec3::factorize(helperA, Gn, true) * 0.5 / Gn_len2;
			nCoefs[1] = Vec3::factorize(helperB, Gn, true) * 0.5 / Gn_len2;
			nCoefs[2] = Vec3::factorize(helperC, Gn, true) / Gn_len2;
			nCoefs[3] = B - 2 * nCoefs[0] * u - nCoefs[2] * v;
			nCoefs[4] = D - 2 * nCoefs[1] * v - nCoefs[2] * u;
			nCoefs[5] = n - nCoefs[0] * u * u - nCoefs[1] * v * v - nCoefs[2] * u * v - nCoefs[3] * u - nCoefs[4] * v;
		}

		// U, V Coefs
		{
			Real
				A = Vec3::factorize(Gm, Fu, false),
				B = Vec3::factorize(Gm, Fv, false),
				C = Vec3::factorize(Gn, Fu, false),
				D = Vec3::factorize(Gn, Fv, false);

			// C1
			{
				Vec3 terms[3];
				terms[0] = Fuu * A * A;
				terms[1] = Fvv * B * B;
				terms[2] = Fuv * 2 * A * B;

				Vec3 res;
				res = Gmm - terms[0] - terms[1] - terms[2];
				uCoefs[0] = Vec3::factorize(res, Fu, false) * 0.5;
				vCoefs[0] = Vec3::factorize(res, Fv, false) * 0.5;
			}
			// C2
			{
				Vec3 terms[3];
				terms[0] = Fuu * C * C;
				terms[1] = Fvv * D * D;
				terms[2] = Fuv * 2 * C * D;

				Vec3 res;
				res = Gnn - terms[0] - terms[1] - terms[2];
				uCoefs[1] = Vec3::factorize(res, Fu, false) * 0.5;
				vCoefs[1] = Vec3::factorize(res, Fv, false) * 0.5;
			}
			// C3
			{
				Vec3 terms[3];
				terms[0] = Fuu * A * C;
				terms[1] = Fvv * B * D;// @TODO : Maybe C * D ?
				terms[2] = Fuv * (A * D + B * C);

				Vec3 res;
				res = Gmn - terms[0] - terms[1] - terms[2];
				uCoefs[2] = Vec3::factorize(res, Fu, false);
				vCoefs[2] = Vec3::factorize(res, Fv, false);
			}
			// C4
			uCoefs[3] = A - 2 * uCoefs[0] * m - uCoefs[2] * n;
			vCoefs[3] = B - 2 * vCoefs[0] * m - vCoefs[2] * n;
			// C5
			uCoefs[4] = C - 2 * uCoefs[1] * n - uCoefs[2] * m;
			vCoefs[4] = D - 2 * vCoefs[1] * n - vCoefs[2] * m;
			// C6
			uCoefs[5] = u - uCoefs[0] * m * m - uCoefs[1] * n * n - uCoefs[2] * m * n - uCoefs[3] * m - uCoefs[4] * n;
			vCoefs[5] = v - vCoefs[0] * m * m - vCoefs[1] * n * n - vCoefs[2] * m * n - vCoefs[3] * m - vCoefs[4] * n;
		}
	}
	void TorusApprox::Mapping::calMN(Real u, Real v, Real& m, Real& n) const {
		m = evaluate(mCoefs, u, v);
		n = evaluate(nCoefs, u, v);
	}
	void TorusApprox::Mapping::calUV(Real m, Real n, Real& u, Real& v) const {
		u = evaluate(uCoefs, m, n);
		v = evaluate(vCoefs, m, n);
	}
	Real TorusApprox::Mapping::evaluate(const std::array<Real, 6>& c, Real u, Real v) {
		return c[0] * u * u + c[1] * v * v + c[2] * u * v + c[3] * u + c[4] * v + c[5];
	}
	static void updateDomain(Domain& domain, Real value) {
		Real beg = domain.beg(), end = domain.end();
		if (value < beg) beg = value;
		if (value > end) end = value;
		domain.set(beg, end);
	}

	bool TorusApprox::Mapping::calDomainM(const Domain& uDomain, const Domain& vDomain, piDomain& mDomain) {
		// Check [ Mu ]
		bool posMu;
		Real Mu00 = 2 * mCoefs[0] * uDomain.beg() + mCoefs[2] * vDomain.beg() + mCoefs[3];
		Real det;
		if (Mu00 > 0) {
			if(mCoefs[0] > 0 && mCoefs[2] > 0)
				posMu = true;
			else if (mCoefs[0] > 0) {
				det = 2 * mCoefs[0] * uDomain.beg() + mCoefs[2] * vDomain.end() + mCoefs[3];
				if (det < 0)
					return false;
				posMu = true;
			}
			else if (mCoefs[2] > 0) {
				det = 2 * mCoefs[0] * uDomain.end() + mCoefs[2] * vDomain.beg() + mCoefs[3];
				if (det < 0)
					return false;
				posMu = true;
			}
			else {
				det = 2 * mCoefs[0] * uDomain.end() + mCoefs[2] * vDomain.end() + mCoefs[3];
				if (det < 0)
					return false;
				posMu = true;
			}
		}
		else if (Mu00 < 0) {
			if (mCoefs[0] < 0 && mCoefs[2] < 0)
				posMu = false;
			else if (mCoefs[0] < 0) {
				det = 2 * mCoefs[0] * uDomain.beg() + mCoefs[2] * vDomain.end() + mCoefs[3];
				if (det > 0)
					return false;
				posMu = false;
			}
			else if (mCoefs[2] < 0) {
				det = 2 * mCoefs[0] * uDomain.end() + mCoefs[2] * vDomain.beg() + mCoefs[3];
				if (det > 0)
					return false;
				posMu = false;
			}
			else {
				det = 2 * mCoefs[0] * uDomain.end() + mCoefs[2] * vDomain.end() + mCoefs[3];
				if (det > 0)
					return false;
				posMu = false;
			}
		}
		else
			return false;

		// Check [ Mv ]
		bool posMv;
		Real Mv00 = 2 * mCoefs[1] * vDomain.beg() + mCoefs[2] * uDomain.beg() + mCoefs[4];
		if (Mv00 > 0) {
			if (mCoefs[1] > 0 && mCoefs[2] > 0)
				posMv = true;
			else if (mCoefs[1] > 0) {
				det = 2 * mCoefs[1] * vDomain.beg() + mCoefs[2] * uDomain.end() + mCoefs[4];
				if (det < 0)
					return false;
				posMv = true;
			}
			else if (mCoefs[2] > 0) {
				det = 2 * mCoefs[0] * vDomain.end() + mCoefs[2] * uDomain.beg() + mCoefs[4];
				if (det < 0)
					return false;
				posMv = true;
			}
			else {
				det = 2 * mCoefs[0] * vDomain.end() + mCoefs[2] * uDomain.end() + mCoefs[4];
				if (det < 0)
					return false;
				posMv = true;
			}
		}
		else if (Mv00 < 0) {
			if (mCoefs[1] < 0 && mCoefs[2] < 0)
				posMv = false;
			else if (mCoefs[1] < 0) {
				det = 2 * mCoefs[1] * vDomain.beg() + mCoefs[2] * uDomain.end() + mCoefs[4];
				if (det > 0)
					return false;
				posMv = false;
			}
			else if (mCoefs[2] < 0) {
				det = 2 * mCoefs[1] * vDomain.end() + mCoefs[2] * uDomain.beg() + mCoefs[4];
				if (det > 0)
					return false;
				posMv = false;
			}
			else {
				det = 2 * mCoefs[1] * vDomain.end() + mCoefs[2] * uDomain.end() + mCoefs[4];
				if (det > 0)
					return false;
				posMv = false;
			}
		}
		else
			return false;

		// Get min, max value
		Real min, max;
		if (posMu && posMv) {
			min = evaluate(mCoefs, uDomain.beg(), vDomain.beg());
			max = evaluate(mCoefs, uDomain.end(), vDomain.end());
		}
		else if (posMu) {
			min = evaluate(mCoefs, uDomain.beg(), vDomain.end());
			max = evaluate(mCoefs, uDomain.end(), vDomain.beg());
		}
		else if (posMv) {
			min = evaluate(mCoefs, uDomain.end(), vDomain.beg());
			max = evaluate(mCoefs, uDomain.beg(), vDomain.end());
		}
		else {
			min = evaluate(mCoefs, uDomain.end(), vDomain.end());
			max = evaluate(mCoefs, uDomain.beg(), vDomain.beg());
		}
		if (max - min < PI20)
			mDomain.set(min, max);
		else
			mDomain.set(0, PI20);
		return true;
	}
	bool TorusApprox::Mapping::calDomainN(const Domain& uDomain, const Domain& vDomain, piDomain& nDomain) {
		// Check [ Nu ]
		bool posNu;
		Real Nu00 = 2 * nCoefs[0] * uDomain.beg() + nCoefs[2] * vDomain.beg() + nCoefs[3];
		Real det;
		if (Nu00 > 0) {
			if (nCoefs[0] > 0 && nCoefs[2] > 0)
				posNu = true;
			else if (nCoefs[0] > 0) {
				det = 2 * nCoefs[0] * uDomain.beg() + nCoefs[2] * vDomain.end() + nCoefs[3];
				if (det < 0)
					return false;
				posNu = true;
			}
			else if (nCoefs[2] > 0) {
				det = 2 * nCoefs[0] * uDomain.end() + nCoefs[2] * vDomain.beg() + nCoefs[3];
				if (det < 0)
					return false;
				posNu = true;
			}
			else {
				det = 2 * nCoefs[0] * uDomain.end() + nCoefs[2] * vDomain.end() + nCoefs[3];
				if (det < 0)
					return false;
				posNu = true;
			}
		}
		else if (Nu00 < 0) {
			if (nCoefs[0] < 0 && nCoefs[2] < 0)
				posNu = false;
			else if (nCoefs[0] < 0) {
				det = 2 * nCoefs[0] * uDomain.beg() + nCoefs[2] * vDomain.end() + nCoefs[3];
				if (det > 0)
					return false;
				posNu = false;
			}
			else if (nCoefs[2] < 0) {
				det = 2 * nCoefs[0] * uDomain.end() + nCoefs[2] * vDomain.beg() + nCoefs[3];
				if (det > 0)
					return false;
				posNu = false;
			}
			else {
				det = 2 * nCoefs[0] * uDomain.end() + nCoefs[2] * vDomain.end() + nCoefs[3];
				if (det > 0)
					return false;
				posNu = false;
			}
		}
		else
			return false;

		// Check [ Nv ]
		bool posNv;
		Real Nv00 = 2 * nCoefs[1] * vDomain.beg() + nCoefs[2] * uDomain.beg() + nCoefs[4];
		if (Nv00 > 0) {
			if (nCoefs[1] > 0 && nCoefs[2] > 0)
				posNv = true;
			else if (nCoefs[1] > 0) {
				det = 2 * nCoefs[1] * vDomain.beg() + nCoefs[2] * uDomain.end() + nCoefs[4];
				if (det < 0)
					return false;
				posNv = true;
			}
			else if (nCoefs[2] > 0) {
				det = 2 * nCoefs[1] * vDomain.end() + nCoefs[2] * uDomain.beg() + nCoefs[4];
				if (det < 0)
					return false;
				posNv = true;
			}
			else {
				det = 2 * nCoefs[1] * vDomain.end() + nCoefs[2] * uDomain.end() + nCoefs[4];
				if (det < 0)
					return false;
				posNv = true;
			}
		}
		else if (Nv00 < 0) {
			if (nCoefs[1] < 0 && nCoefs[2] < 0)
				posNv = false;
			else if (nCoefs[1] < 0) {
				det = 2 * nCoefs[1] * vDomain.beg() + nCoefs[2] * uDomain.end() + nCoefs[4];
				if (det > 0)
					return false;
				posNv = false;
			}
			else if (nCoefs[2] < 0) {
				det = 2 * nCoefs[1] * vDomain.end() + nCoefs[2] * uDomain.beg() + nCoefs[4];
				if (det > 0)
					return false;
				posNv = false;
			}
			else {
				det = 2 * nCoefs[1] * vDomain.end() + nCoefs[2] * uDomain.end() + nCoefs[4];
				if (det > 0)
					return false;
				posNv = false;
			}
		}
		else
			return false;

		// Get min, max value
		Real min, max;
		if (posNu && posNv) {
			min = evaluate(nCoefs, uDomain.beg(), vDomain.beg());
			max = evaluate(nCoefs, uDomain.end(), vDomain.end());
		}
		else if (posNu) {
			min = evaluate(nCoefs, uDomain.beg(), vDomain.end());
			max = evaluate(nCoefs, uDomain.end(), vDomain.beg());
		}
		else if (posNv) {
			min = evaluate(nCoefs, uDomain.end(), vDomain.beg());
			max = evaluate(nCoefs, uDomain.beg(), vDomain.end());
		}
		else {
			min = evaluate(nCoefs, uDomain.end(), vDomain.end());
			max = evaluate(nCoefs, uDomain.beg(), vDomain.beg());
		}
		if (max - min < PI20)
			nDomain.set(min, max);
		else
			nDomain.set(0, PI20);
		return true;
	}

	/*Domain TorusApprox::Mapping::calDomain(int mn, const Domain& uDomain, const Domain& vDomain) {
		const auto& coefs = (mn == 0) ? mCoefs : nCoefs;

		Domain domain;
		Real
			c1 = coefs[0], c2 = coefs[1], c3 = coefs[2], c4 = coefs[3], c5 = coefs[4], c6 = coefs[5];
		domain.first = 1e+10;
		domain.second = -1e+10;
		// 1. Evaluate corner values.
		Real
			val;
		{
			val = evaluate(coefs, uDomain.first, vDomain.first); updateDomain(domain, val);
			val = evaluate(coefs, uDomain.first, vDomain.second); updateDomain(domain, val);
			val = evaluate(coefs, uDomain.second, vDomain.second); updateDomain(domain, val);
			val = evaluate(coefs, uDomain.second, vDomain.first); updateDomain(domain, val);
		}
		// 2. Evaluate edge values.
		{
			// [u, v0], [u, v1] edge.
			if (c1 != 0.0) {
				double
					tu0 = (-c3 * vDomain.first - c4) / (2 * c1),
					tu1 = (-c3 * vDomain.second - c4) / (2 * c1);
				if (tu0 >= uDomain.first && tu0 <= uDomain.second) {
					val = evaluate(coefs, tu0, vDomain.first);
					updateDomain(domain, val);
				}
				if (tu1 >= uDomain.first && tu1 <= uDomain.second) {
					val = evaluate(coefs, tu1, vDomain.second);
					updateDomain(domain, val);
				}
			}
			// [u0, v], [u1, v] edge.
			if (c2 != 0.0) {
				double
					tv0 = (-c3 * uDomain.first - c5) / (2 * c2),
					tv1 = (-c3 * uDomain.second - c5) / (2 * c2);
				if (tv0 >= vDomain.first && tv0 <= vDomain.second) {
					val = evaluate(coefs, uDomain.first, tv0);
					updateDomain(domain, val);
				}
				if (tv1 >= vDomain.first && tv1 <= vDomain.second) {
					val = evaluate(coefs, uDomain.second, tv1);
					updateDomain(domain, val);
				}
			}
		}

		// 3. Evaluate inner values.
		{
			Real
				det = 4 * c1 * c2 - c3 * c3,
				eps = 1e-10;
			if (fabs(det) > eps) {	// @BUGFIX : Do not have to care about det == 0 case
				double
					tu = (-2 * c2 * c4 + c3 * c5) / det,
					tv = (2 * c1 * tu + c4) / (-c3);
				if (tu >= uDomain.first && tu <= uDomain.second && tv >= vDomain.first && tv <= vDomain.second) {
					val = evaluate(coefs, tu, tv);
					updateDomain(domain, val);
				}
			}
		}
		if (mn == 0) {
			Real tmp = (fabs(domain.first) > fabs(domain.second)) ? fabs(domain.first) : fabs(domain.second);
			domain.first = -tmp;
			domain.second = tmp;
		}
		else {
			Real tmp = (fabs(domain.first - n0) > fabs(domain.second - n0)) ? fabs(domain.first - n0) : fabs(domain.second - n0);
			domain.first = n0 - tmp;
			domain.second = n0 + tmp;
		}
		return domain;
	}*/
	void TorusApprox::Mapping::calPositionErrorUpperBound(const TorusPatch& patch, const Domain& uDomain, const Domain& vDomain, Real& N1, Real& N2, Real& N3, Real& N4) const {
		Real Mu, Mv, Nu, Nv, Muu, Muv, Mvv, Nuu, Nuv, Nvv;
		{
			Real tMu;
			Mu = fabs(2.0 * mCoefs[0] * uDomain.beg() + mCoefs[2] * vDomain.beg() + mCoefs[3]);

			tMu = fabs(2.0 * mCoefs[0] * uDomain.beg() + mCoefs[2] * vDomain.end() + mCoefs[3]);
			if (tMu > Mu) Mu = tMu;

			tMu = fabs(2.0 * mCoefs[0] * uDomain.end() + mCoefs[2] * vDomain.beg() + mCoefs[3]);
			if (tMu > Mu) Mu = tMu;

			tMu = fabs(2.0 * mCoefs[0] * uDomain.end() + mCoefs[2] * vDomain.end() + mCoefs[3]);
			if (tMu > Mu) Mu = tMu;
		}
		{
			Real tMv;
			Mv = fabs(2.0 * mCoefs[1] * vDomain.beg() + mCoefs[2] * uDomain.beg() + mCoefs[4]);

			tMv = fabs(2.0 * mCoefs[1] * vDomain.beg() + mCoefs[2] * uDomain.end() + mCoefs[4]);
			if (tMv > Mv) Mv = tMv;

			tMv = fabs(2.0 * mCoefs[1] * vDomain.end() + mCoefs[2] * uDomain.beg() + mCoefs[4]);
			if (tMv > Mv) Mv = tMv;

			tMv = fabs(2.0 * mCoefs[1] * vDomain.end() + mCoefs[2] * uDomain.end() + mCoefs[4]);
			if (tMv > Mv) Mv = tMv;
		}
		{
			Real tNu;
			Nu = fabs(2.0 * nCoefs[0] * uDomain.beg() + nCoefs[2] * vDomain.beg() + nCoefs[3]);

			tNu = fabs(2.0 * nCoefs[0] * uDomain.beg() + nCoefs[2] * vDomain.end() + nCoefs[3]);
			if (tNu > Nu) Nu = tNu;

			tNu = fabs(2.0 * nCoefs[0] * uDomain.end() + nCoefs[2] * vDomain.beg() + nCoefs[3]);
			if (tNu > Nu) Nu = tNu;

			tNu = fabs(2.0 * nCoefs[0] * uDomain.end() + nCoefs[2] * vDomain.end() + nCoefs[3]);
			if (tNu > Nu) Nu = tNu;
		}
		{
			Real tNv;
			Nv = fabs(2.0 * nCoefs[1] * vDomain.beg() + nCoefs[2] * uDomain.beg() + nCoefs[4]);

			tNv = fabs(2.0 * nCoefs[1] * vDomain.beg() + nCoefs[2] * uDomain.end() + nCoefs[4]);
			if (tNv > Nv) Nv = tNv;

			tNv = fabs(2.0 * nCoefs[1] * vDomain.end() + nCoefs[2] * uDomain.beg() + nCoefs[4]);
			if (tNv > Nv) Nv = tNv;

			tNv = fabs(2.0 * nCoefs[1] * vDomain.end() + nCoefs[2] * uDomain.end() + nCoefs[4]);
			if (tNv > Nv) Nv = tNv;
		}

		Muu = fabs(2.0 * mCoefs[0]);
		Muv = fabs(mCoefs[2]);
		Mvv = fabs(2.0 * mCoefs[1]);

		Nuu = fabs(2.0 * nCoefs[0]);
		Nuv = fabs(nCoefs[2]);
		Nvv = fabs(2.0 * nCoefs[1]);

		Real Gmmm, Gmmn, Gmnn, Gnnn, Gmm, Gmn, Gnn;
		piDomain mDomain = patch.uDomain, nDomain = patch.vDomain;
		{
			// @TODO : Width assumption
			if (n0 == 0) {
				Gmmm = patch.majorRadius + patch.minorRadius;
				if (nDomain.width() > PI)
					Gmmn = patch.minorRadius;
				else
					Gmmn = patch.minorRadius * fabs(sin(nDomain.end()));
				Gmnn = patch.minorRadius;
				Gnnn = patch.minorRadius;
			}
			else {
				Gmmm = patch.majorRadius + patch.minorRadius * cos(nDomain.beg());
				if (nDomain.width() > PI)
					Gmmn = patch.minorRadius;
				else
					Gmmn = patch.minorRadius * fabs(sin(nDomain.end()));
				Gmnn = patch.minorRadius;
				Gnnn = patch.minorRadius;
			}
			Gmm = Gmmm;
			Gmn = Gmmn;
			Gnn = Gnnn;
		}

		N1 = (Gmmm * Mu * Mu * Mu) + 3.0 * (Gmmn * Mu * Mu * Nu) + 3.0 * (Gmnn * Mu * Nu * Nu) + (Gnnn * Nu * Nu * Nu)
			+ 3.0 * (Gmm * Mu * Muu) + 3.0 * (Gnn * Nu * Nuu) + 3.0 * (Gmn * (Muu * Nu + Mu * Nuu));
		N2 = (Gmmm * Mu * Mu * Mv) + (Gmmn * (Mu * Mu * Nv + 2.0 * Mu * Nu * Mv)) + (Gmnn * (Mv * Nu * Nu + 2.0 * Mu * Nu * Nv))
			+ (Gnnn * Nu * Nu * Nv) + (Gmm * (2.0 * Mu * Muv + Muu * Mv)) + (Gnn * (2.0 * Nu * Nuv + Nuu * Nv))
			+ (Gmn * (2.0 * Muv * Nu + 2.0 * Nuv * Mu + Muu * Nv + Nuu * Mv));
		N3 = (Gmmm * Mv * Mv * Mu) + (Gmmn * (Mv * Mv * Nu + 2.0 * Mv * Nv * Mu)) + (Gmnn * (Mu * Nv * Nv + 2.0 * Mv * Nv * Nu))
			+ (Gnnn * Nv * Nv * Nu) + (Gmm * (2.0 * Mv * Muv + Mvv * Mu)) + (Gnn * (2.0 * Nv * Nuv + Nvv * Nu))
			+ (Gmn * (2.0 * Muv * Nv + 2.0 * Nuv * Mv + Mvv * Nu + Nvv * Mu));
		N4 = (Gmmm * Mv * Mv * Mv) + 3.0 * (Gmmn * Mv * Mv * Nv) + 3.0 * (Gmnn * Mv * Nv * Nv) + (Gnnn * Nv * Nv * Nv)
			+ 3.0 * (Gmm * Mv * Mvv) + 3.0 * (Gnn * Nv * Nvv) + 3.0 * (Gmn * (Mvv * Nv + Mv * Nvv));
	}

	// TorusApprox
	TorusApprox TorusApprox::create(const SurfaceInfo& surface) {
		TorusApprox ta;
		Vec3 point, Fu, Fv, Fuu, Fuv, Fvv, normal;
		point = surface.F;
		Fu = surface.Fu;
		Fv = surface.Fv;
		Fuu = surface.Fuu;
		Fuv = surface.Fuv;
		Fvv = surface.Fvv;
		normal = Fu.cross(Fv);
		normal.normalize();

		Real k1, k2;	// Principal curvatures
		Vec3 w1, w2;	// Principal directions

		// Calculate principal curvatures and directions
		{
			Real
				E = Fu.dot(Fu),
				F = Fu.dot(Fv),
				G = Fv.dot(Fv),
				denom = sqrt(E * G - F * F),
				e = Vec3::Tcross(Fu, Fv, Fuu) / denom,
				f = Vec3::Tcross(Fu, Fv, Fuv) / denom,
				g = Vec3::Tcross(Fu, Fv, Fvv) / denom;
			denom = E * G - F * F;
			Real
				K = (e * g - f * f) / denom,
				H = 0.5 * (e * G - 2.0 * f * F + g * E) / denom;
			Real
				tmp = sqrt(H * H - K);
			k1 = H + tmp;
			k2 = H - tmp;
			Real
				a11 = (f * F - e * G) / denom,
				a12 = (g * F - f * G) / denom,
				a21 = (e * F - f * E) / denom,
				a22 = (f * F - g * E) / denom;
			Real
				eps = 1e-5;	// @TODO : Have to set proper value for this
			if (fabs(a12) < eps && fabs(a21) < eps) {
				// Set principal direction properly
				Real
					a11_k1 = (a11 + k1) * (a11 + k1),
					a11_k2 = (a11 + k2) * (a11 + k2);	// Compare a11 + k1, k2. Find k s.t. a11 = -k
				if (a11_k1 < a11_k2) {
					// Xu direction has k1 curvature
					w1 = Fu;
					w2 = Fv;
				}
				else {
					// Xu direction has k2 curvature
					w1 = Fv;
					w2 = Fu;
				}
			}
			else {
				w1 = Fu + Fv * ((-k1 - a11) / a12);
				w2 = Fu + Fv * ((-k2 - a11) / a12);
			}
			w1.normalize();
			w2.normalize();
		}
		const static Real curvatureEps = 1e-7;	// Have to choose value that is not numerically unstable
		// parabolic : @TODO : To avoid parabolic case, we enlarge curvature a little
		// This may incur wrong error estimation, but we expect it would rarely happen
		if (fabs(k1) < curvatureEps)
			k1 = (k1 < 0 ? -curvatureEps : curvatureEps);
		if (fabs(k2) < curvatureEps)
			k2 = (k2 < 0 ? -curvatureEps : curvatureEps);
		/*if (k1 == 0 || k2 == 0)
			throw(std::runtime_error("Curvature zero"));*/

		// Map surface(u, v) to proper torus parameters
		Real m, n;
		if (k1 * k2 < 0) { // hyperbolic
			m = 0;
			n = PI;
		}
		else {	// elliptic
			m = 0;
			n = 0;
		}
		if (fabs(k1) > fabs(k2)) {
			Real tmp = k2;
			k2 = k1;
			k1 = tmp;
			std::swap(w1, w2);
		}
		if (k1 > 0) {
			normal *= -1;
			k1 *= -1.0;
			k2 *= -1.0;
		}
		Vec3
			torusCenterAdd, torusCenter, torusAxis = w2;
		torusCenterAdd = normal / k1;
		torusCenter = point + torusCenterAdd;

		ta.patch.majorRadius = -1.0 / k1 + 1.0 / k2,
		ta.patch.minorRadius = 1.0 / fabs(k2);
		ta.setTransform(torusCenter, torusAxis);

		// Rotate to make [point] correspond to (0, 0) or (0, PI)
		// [originalPoint] must go to [point]
		Vec3
			originalPoint, sub1, sub2, tmpAxis;
		originalPoint = ta.transform.apply(ta.patch.evaluate(m, n));
		sub1 = point - torusCenter;
		sub2 = originalPoint - torusCenter;
		sub1.normalize();
		sub2.normalize();
		tmpAxis = sub2.cross(sub1);

		Vec3 axisEnd;
		axisEnd = torusCenter + torusAxis;
		if (tmpAxis.dot(torusAxis) > 0)
			ta.transform.rotate(torusCenter, axisEnd, acos(sub2.dot(sub1)));
		else
			ta.transform.rotate(torusCenter, axisEnd, -acos(sub2.dot(sub1)));
		ta.iTransform = ta.transform.inverse();


		// Set [H].
		ta.mapping.set(surface, ta, m, n);


		// Set Torus Patch Domain
		{
			piDomain mDomain, nDomain;
			bool validM = ta.mapping.calDomainM(surface.uDomain, surface.vDomain, mDomain);
			bool validN = ta.mapping.calDomainN(surface.uDomain, surface.vDomain, nDomain);
			if (validM && validN) {
				ta.patch.uDomain = mDomain;
				ta.patch.vDomain = nDomain;
				ta.validPatchDomain = true;
			}
			else {
				ta.patch.uDomain.set(0, PI20);
				ta.patch.vDomain.set(0, PI20);
				ta.validPatchDomain = false;
			}
		}

		// Set Error Bound
		Real L1, L2;
		L1 = surface.uDomain.width() * 0.5;
		L2 = surface.vDomain.width() * 0.5;

		Real M1, M2, M3, M4;
		Real N1, N2, N3, N4;
		M1 = surface.M1;
		M2 = surface.M2;
		M3 = surface.M3;
		M4 = surface.M4;

		ta.mapping.calPositionErrorUpperBound(ta.patch, surface.uDomain, surface.vDomain, N1, N2, N3, N4);
		ta.positionError = (2.0 / 3.0) * ((L1 * L1 * L1) * (M1 + N1) + 3 * (L1 * L1 * L2) * (M2 + N2) + 3 * (L1 * L2 * L2) * (M3 + N3) + (L2 * L2 * L2) * (M4 + N4));
		return ta;
	}
	TorusApprox::Ptr TorusApprox::createPtr(const SurfaceInfo& surface) {
		TorusApprox ta = create(surface);
		return std::make_shared<TorusApprox>(ta);
	}
	void TorusApprox::setTransform(const Vec3& center, const Vec3& axis) {
		transform.clear();

		transform.translate(center);
		// Original axis is (0, 0, 1), and we need to rotate it to be same as given axis
		Vec3
			originalAxis{ 0, 0, 1 },
			targetAxis = axis,
			axisHelper;
		targetAxis.normalize();
		if (targetAxis == originalAxis)
			return;
		else if (targetAxis[2] == -1.0)
			axisHelper = { 1, 0, 0 };
		else {
			axisHelper = originalAxis.cross(targetAxis); // Helper axis to make axis same
			axisHelper.normalize();
		}
		double
			radian = acos(originalAxis.dot(targetAxis));
		Vec3 axisEnd;
		axisEnd = center + axisHelper;
		transform.rotate(center, axisEnd, radian);
	}
}