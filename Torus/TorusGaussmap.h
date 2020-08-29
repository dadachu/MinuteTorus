/*
 *******************************************************************************************
 * Author	: Sang Hyun Son
 * Email	: shh1295@gmail.com
 * Github	: github.com/SonSang
 *******************************************************************************************
 */

#ifndef __MN_TORUS_GAUSSMAP_H__
#define __MN_TORUS_GAUSSMAP_H__

#ifdef _MSC_VER
#pragma once
#endif

#include "Torus.h"
#include "../Gaussmap/Gaussmap.h"

namespace MN {
	class TorusPatchGaussmap {
	public:
		TorusPatch patch;

		// Domains that partition entire piDomain
		const static piDomain h1;
		const static piDomain h2;
		/* 
		 * Four types of gaussmap is possible for a single torus patch 
		 *  0) outwardH1 : Gaussmap generated by outward normals where [ v ] is in [ -0.5PI ~ 0.5PI ]
		 *  1) outwardH2 : Gaussmap generated by outward normals where [ v ] is in [ 0.5PI ~ 1.5PI ]
		 *  2) inwardH1 : Gaussmap generated by inward normals where [ v ] is in [ -0.5PI ~ 0.5PI ]
		 *  3) inwardH2 : Gaussmap generated by inward normals where [ v ] is in [ 0.5PI ~ 1.5PI ]
		 */
		Gaussmap gaussmap[4] = { Gaussmap::create(), Gaussmap::create(), Gaussmap::create(), Gaussmap::create() };
		Gaussmap iGaussmap[4] = { Gaussmap::create(), Gaussmap::create(), Gaussmap::create(), Gaussmap::create() };		// Inverted gaussmaps ( in both U, V parameters )
		bool validGaussmap[4];

		// Whether or not U inversion occurs for each of the gaussmaps
		const static bool uInversion[4];
		const static bool iuInversion[4];		// Inverted gaussmap version
	private:
		TorusPatchGaussmap() = default;
	public:
		inline static piDomain invertGaussmapDomainU(const piDomain& uDomain) {
			return piDomain::create(uDomain.beg() + PI, uDomain.end() + PI);
		}
		inline static piDomain invertGaussmapDomainV(const piDomain& vDomain) {
			return piDomain::create(PI - vDomain.end(), PI - vDomain.beg());
		}
		inline static Gaussmap invertGaussmap(const Gaussmap& gaussmap) {
			piDomain uDomain = invertGaussmapDomainU(gaussmap.uDomain);
			piDomain vDomain = invertGaussmapDomainV(gaussmap.vDomain);
			return Gaussmap::create(uDomain, vDomain);
		}
		
		inline static TorusPatchGaussmap create(const TorusPatch& patch) {
			TorusPatchGaussmap tpg;
			tpg.patch = patch;

			for (int i = 0; i < 4; i++)
				tpg.gaussmap[i].uDomain = (uInversion[i] ? invertGaussmapDomainU(patch.uDomain) : patch.uDomain);

			bool vIncludePI05 = patch.vDomain.has(PI05);
			bool vIncludePI15 = patch.vDomain.has(PI15);
			if (vIncludePI05 && vIncludePI15) {
				for (int i = 0; i < 4; i++)
					tpg.validGaussmap[i] = true;

				// @TODO : In this case, just set [ vDomain ] of every gaussmap to [ 0, PI ]...
				for (int i = 0; i < 4; i++)
					tpg.gaussmap[i].vDomain.set(0, PI);
			}
			else if (vIncludePI05) {
				for (int i = 0; i < 4; i++)
					tpg.validGaussmap[i] = true;

				// h1v : Boundary v parameter in h1 domain
				// h2v : Boundary v parameter in h2 domain
				Real h1v, h2v;
				if (h1.has(patch.vDomain.beg())) {
					h1v = piDomain::regularize(patch.vDomain.beg());
					h2v = piDomain::regularize(patch.vDomain.end());
				}
				else {
					h1v = piDomain::regularize(patch.vDomain.end());
					h2v = piDomain::regularize(patch.vDomain.beg());
				}
				if (h1v >= PI15)
					h1v -= PI20;

				tpg.gaussmap[0].vDomain.set(0, PI05 - h1v);
				tpg.gaussmap[1].vDomain.set(0, h2v - PI05);
				tpg.gaussmap[2].vDomain = invertGaussmapDomainV(tpg.gaussmap[0].vDomain);
				tpg.gaussmap[3].vDomain = invertGaussmapDomainV(tpg.gaussmap[1].vDomain);
			}
			else if (vIncludePI15) {
				for (int i = 0; i < 4; i++)
					tpg.validGaussmap[i] = true;

				// h1v : Boundary v parameter in h1 domain
				// h2v : Boundary v parameter in h2 domain
				Real h1v, h2v;
				if (h1.has(patch.vDomain.beg())) {
					h1v = piDomain::regularize(patch.vDomain.beg());
					h2v = piDomain::regularize(patch.vDomain.end());
				}
				else {
					h1v = piDomain::regularize(patch.vDomain.end());
					h2v = piDomain::regularize(patch.vDomain.beg());
				}
				if (h1v >= PI15)
					h1v -= PI20;

				tpg.gaussmap[0].vDomain.set(h1v + PI05, PI);
				tpg.gaussmap[1].vDomain.set(PI15 - h2v, PI);
				tpg.gaussmap[2].vDomain = invertGaussmapDomainV(tpg.gaussmap[0].vDomain);
				tpg.gaussmap[3].vDomain = invertGaussmapDomainV(tpg.gaussmap[1].vDomain);
			}
			else {
				if (h1.has(patch.vDomain.beg())) {
					tpg.validGaussmap[0] = true;
					tpg.validGaussmap[1] = false;
					tpg.validGaussmap[2] = true;
					tpg.validGaussmap[3] = false;

					Real vBegin, vEnd;
					vBegin = piDomain::regularize(patch.vDomain.beg());
					vBegin = (vBegin >= PI15 ? PI - (vBegin - PI15) : PI05 - vBegin);
					vEnd = piDomain::regularize(patch.vDomain.end());
					vEnd = (vEnd >= PI15 ? PI - (vEnd - PI15) : PI05 - vEnd);

					if (vBegin > vEnd) 
						std::swap(vBegin, vEnd);

					tpg.gaussmap[0].vDomain.set(vBegin, vEnd);
					tpg.gaussmap[2].vDomain = invertGaussmapDomainV(tpg.gaussmap[0].vDomain);
				}
				else {
					tpg.validGaussmap[0] = false;
					tpg.validGaussmap[1] = true;
					tpg.validGaussmap[2] = false;
					tpg.validGaussmap[3] = true;

					Real vBegin, vEnd;
					vBegin = piDomain::regularize(patch.vDomain.beg());
					vBegin -= PI05;
					vEnd = piDomain::regularize(patch.vDomain.end());
					vEnd -= PI05;

					tpg.gaussmap[1].vDomain.set(vBegin, vEnd);
					tpg.gaussmap[3].vDomain = invertGaussmapDomainV(tpg.gaussmap[0].vDomain);
				}
			}

			for (int i = 0; i < 4; i++)
				if(tpg.validGaussmap[i])
					tpg.iGaussmap[i] = invertGaussmap(tpg.gaussmap[i]);
			return tpg;
		}
		inline static TorusPatchGaussmap create() {
			TorusPatchGaussmap tpg;
			for (int i = 0; i < 4; i++)
				tpg.validGaussmap[i] = false;
			return tpg;
		}
	};

	using TPatchGmap = TorusPatchGaussmap;
}

#endif