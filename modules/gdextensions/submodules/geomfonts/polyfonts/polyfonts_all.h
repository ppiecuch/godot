/*************************************************************************/
/*  polyfonts_all.h                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef __POLYFONTS_ALL__
#define __POLYFONTS_ALL__

/*
  Polyfonts is a polygon font drawing library for use with SDL. Any
  TTF font can be converted for use with this library. Contact the
  author for details.

  Copyright (C) 2003 Bob Pendleton

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2.1
  of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307 USA

  If you do not wish to comply with the terms of the LGPL please
  contact the author as other terms are available for a fee.

  Bob Pendleton
  Bob@Pendleton.com
*/

#include "polyfonttypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "fonts/pfOSans16.h"
#include "fonts/pfOSans7.h"
#include "fonts/pfOSans8.h"
#include "fonts/pfOSansBold16.h"
#include "fonts/pfOSansBold7.h"
#include "fonts/pfOSansBold8.h"
#include "fonts/pfOSansBoldItalic16.h"
#include "fonts/pfOSansBoldItalic7.h"
#include "fonts/pfOSansBoldItalic8.h"
#include "fonts/pfOSansItalic16.h"
#include "fonts/pfOSansItalic7.h"
#include "fonts/pfOSansItalic8.h"
#include "fonts/pfOSansMono16.h"
#include "fonts/pfOSansMono7.h"
#include "fonts/pfOSansMono8.h"
#include "fonts/pfOSansMonoBold16.h"
#include "fonts/pfOSansMonoBold7.h"
#include "fonts/pfOSansMonoBold8.h"
#include "fonts/pfOSansMonoBoldItalic16.h"
#include "fonts/pfOSansMonoBoldItalic7.h"
#include "fonts/pfOSansMonoBoldItalic8.h"
#include "fonts/pfOSansMonoItalic16.h"
#include "fonts/pfOSansMonoItalic7.h"
#include "fonts/pfOSansMonoItalic8.h"
#include "fonts/pfOSerif16.h"
#include "fonts/pfOSerif7.h"
#include "fonts/pfOSerif8.h"
#include "fonts/pfOSerifBold16.h"
#include "fonts/pfOSerifBold7.h"
#include "fonts/pfOSerifBold8.h"
#include "fonts/pfPSans16.h"
#include "fonts/pfPSans7.h"
#include "fonts/pfPSans8.h"
#include "fonts/pfPSansBold16.h"
#include "fonts/pfPSansBold7.h"
#include "fonts/pfPSansBold8.h"
#include "fonts/pfPSansBoldItalic16.h"
#include "fonts/pfPSansBoldItalic7.h"
#include "fonts/pfPSansBoldItalic8.h"
#include "fonts/pfPSansItalic16.h"
#include "fonts/pfPSansItalic7.h"
#include "fonts/pfPSansItalic8.h"
#include "fonts/pfPSansMono16.h"
#include "fonts/pfPSansMono7.h"
#include "fonts/pfPSansMono8.h"
#include "fonts/pfPSansMonoBold16.h"
#include "fonts/pfPSansMonoBold7.h"
#include "fonts/pfPSansMonoBold8.h"
#include "fonts/pfPSansMonoBoldItalic16.h"
#include "fonts/pfPSansMonoBoldItalic7.h"
#include "fonts/pfPSansMonoBoldItalic8.h"
#include "fonts/pfPSansMonoItalic16.h"
#include "fonts/pfPSansMonoItalic7.h"
#include "fonts/pfPSansMonoItalic8.h"
#include "fonts/pfPSerif16.h"
#include "fonts/pfPSerif7.h"
#include "fonts/pfPSerif8.h"
#include "fonts/pfPSerifBold16.h"
#include "fonts/pfPSerifBold7.h"
#include "fonts/pfPSerifBold8.h"
#include "fonts/pfSAstrology.h"
#include "fonts/pfSCursive.h"
#include "fonts/pfSCyrillic.h"
#include "fonts/pfSFutura.h"
#include "fonts/pfSFuturaMono.h"
#include "fonts/pfSGothicEnglish.h"
#include "fonts/pfSGothicGerman.h"
#include "fonts/pfSGothicItalian.h"
#include "fonts/pfSGreek.h"
#include "fonts/pfSJapanese.h"
#include "fonts/pfSMarkers.h"
#include "fonts/pfSMathLower.h"
#include "fonts/pfSMathUpper.h"
#include "fonts/pfSMeteorology.h"
#include "fonts/pfSMusic.h"
#include "fonts/pfSRoman.h"
#include "fonts/pfSRomanMono.h"
#include "fonts/pfSScript.h"
#include "fonts/pfSSymbolic.h"
#include "fonts/pfSTimes.h"
#include "fonts/pfSTimesBold.h"
#include "fonts/pfSTimesBoldItalic.h"
#include "fonts/pfSTimesGreek.h"
#include "fonts/pfSTimesItalic.h"

pffont *pfAllFonts[] = {
	&pfOSans16,
	&pfOSans7,
	&pfOSans8,
	&pfOSansBold16,
	&pfOSansBold7,
	&pfOSansBold8,
	&pfOSansBoldItalic16,
	&pfOSansBoldItalic7,
	&pfOSansBoldItalic8,
	&pfOSansItalic16,
	&pfOSansItalic7,
	&pfOSansItalic8,
	&pfOSansMono16,
	&pfOSansMono7,
	&pfOSansMono8,
	&pfOSansMonoBold16,
	&pfOSansMonoBold7,
	&pfOSansMonoBold8,
	&pfOSansMonoBoldItalic16,
	&pfOSansMonoBoldItalic7,
	&pfOSansMonoBoldItalic8,
	&pfOSansMonoItalic16,
	&pfOSansMonoItalic7,
	&pfOSansMonoItalic8,
	&pfOSerif16,
	&pfOSerif7,
	&pfOSerif8,
	&pfOSerifBold16,
	&pfOSerifBold7,
	&pfOSerifBold8,
	&pfPSans16,
	&pfPSans7,
	&pfPSans8,
	&pfPSansBold16,
	&pfPSansBold7,
	&pfPSansBold8,
	&pfPSansBoldItalic16,
	&pfPSansBoldItalic7,
	&pfPSansBoldItalic8,
	&pfPSansItalic16,
	&pfPSansItalic7,
	&pfPSansItalic8,
	&pfPSansMono16,
	&pfPSansMono7,
	&pfPSansMono8,
	&pfPSansMonoBold16,
	&pfPSansMonoBold7,
	&pfPSansMonoBold8,
	&pfPSansMonoBoldItalic16,
	&pfPSansMonoBoldItalic7,
	&pfPSansMonoBoldItalic8,
	&pfPSansMonoItalic16,
	&pfPSansMonoItalic7,
	&pfPSansMonoItalic8,
	&pfPSerif16,
	&pfPSerif7,
	&pfPSerif8,
	&pfPSerifBold16,
	&pfPSerifBold7,
	&pfPSerifBold8,
#ifdef OOFONTS
	&pfPAriosoBold,
	&pfPArioso,
	&pfPChevara,
	&pfPChevarao,
	&pfPCongaBold,
	&pfPConga,
	&pfPHelmetBold,
	&pfPHelmetBoldItalic,
	&pfPHelmetcBold,
	&pfPHelmetcBoldItalic,
	&pfPHelmetc,
	&pfPHelmetcItalic,
	&pfPHelmet,
	&pfPHelmetItalic,
	&pfPStarbats,
	&pfPStarmath,
	&pfPTimmonsBold,
	&pfPTimmonsBoldItalic,
	&pfPTimmons,
	&pfPTimmonsItalic,
#endif
#ifdef RLFONTS
	&pfP1980port,
	&pfP256bytes,
	&pfP6809char,
	&pfPAbberanc,
	&pfPAdriator,
	&pfPAirmolea,
	&pfPAirmole,
	&pfPAirmoleq,
	&pfPAirmoles,
	&pfPAlmonte,
	&pfPAlmontew,
	&pfPAlmosnow,
	&pfPAnglepoi,
	&pfPAnklepan,
	&pfPArnprior,
	&pfPAstronbi,
	&pfPAstronbo,
	&pfPAstronbv,
	&pfPAstronbw,
	&pfPAxaxax,
	&pfPBabyjeep,
	&pfPBaileysc,
	&pfPBaltar,
	&pfPBarbatri,
	&pfPBaveuse3,
	&pfPBaveuse,
	&pfPBeatmygu,
	&pfPBerylibi,
	&pfPBeryliub,
	&pfPBeryliui,
	&pfPBerylium,
	&pfPBetsy2,
	&pfPBetsy,
	&pfPBiting,
	&pfPBitingou,
	&pfPBluebold,
	&pfPBluecond,
	&pfPBluehigd,
	&pfPBluehigh,
	&pfPBluehigl,
	&pfPBorg9,
	&pfPBoron2,
	&pfPBudmob,
	&pfPBudmo,
	&pfPBullpen3,
	&pfPBullpen,
	&pfPBullpeni,
	&pfPBurnstow,
	&pfPButterbe,
	&pfPCarbonbl,
	&pfPCarbonph,
	&pfPCharlesi,
	&pfPChineser,
	&pfPColourba,
	&pfPColourbb,
	&pfPContourg,
	&pfPCoolveti,
	&pfPCounters,
	&pfPCrackman,
	&pfPCranberr,
	&pfPCreditri,
	&pfPCreditva,
	&pfPCreditvb,
	&pfPCreditvi,
	&pfPCreditvz,
	&pfPCretino,
	&pfPCrystalr,
	&pfPCuomotyp,
	&pfPDeftone,
	&pfPDegrassi,
	&pfPDeltahey,
	&pfPDendriti,
	&pfPDienasty,
	&pfPDignity,
	&pfPDirtydoz,
	&pfPDreamobx,
	&pfPDreamorb,
	&pfPDreamori,
	&pfPDreamorp,
	&pfPDroid,
	&pfPDuality,
	&pfPDyspepsi,
	&pfPEarwigfa,
	&pfPEchec,
	&pfPEcheci,
	&pfPEchei,
	&pfPEchelon,
	&pfPEdenmb,
	&pfPEdenm,
	&pfPEdenmi,
	&pfPEdmundis,
	&pfPEdmunds,
	&pfPEfflanti,
	&pfPEfflb,
	&pfPEfflbi,
	&pfPEffli,
	&pfPEfflores,
	&pfPElectbgi,
	&pfPElectbgu,
	&pfPElectblu,
	&pfPElectroh,
	&pfPEngeboit,
	&pfPEngebold,
	&pfPEngeexbi,
	&pfPEngeexbo,
	&pfPEngeexit,
	&pfPEngeexpa,
	&pfPEngeital,
	&pfPEngeregu,
	&pfPEnnobled,
	&pfPEthnocen,
	&pfPEuphorig,
	&pfPFabian,
	&pfPFadgod,
	&pfPFailed,
	&pfPFakerece,
	&pfPFluoride,
	&pfPFoo,
	&pfPForgotbi,
	&pfPForgottb,
	&pfPForgotte,
	&pfPForgotti,
	&pfPForgotts,
	&pfPFragileb,
	&pfPFrozen,
	&pfPGhostmea,
	&pfPGoldengi,
	&pfPGoodfisb,
	&pfPGoodfisc,
	&pfPGoodfish,
	&pfPGoodfisi,
	&pfPGoodtime,
	&pfPGotnohea,
	&pfPGraffiti,
	&pfPGreenfuz,
	&pfPGroovygh,
	&pfPGuanine,
	&pfPGunplay3,
	&pfPGunplay,
	&pfPGyparody,
	&pfPHeartswe,
	&pfPHeavyhea,
	&pfPHeck,
	&pfPHellolar,
	&pfPHemihead,
	&pfPHomeswee,
	&pfPHomesweo,
	&pfPHots,
	&pfPHurryup,
	&pfPHuskysta,
	&pfPHydrogen,
	&pfPInflamma,
	&pfPInterpla,
	&pfPIomanoid,
	&pfPJoystix,
	&pfPJunegull,
	&pfPKenycb,
	&pfPKenycbi,
	&pfPKenyc,
	&pfPKenyci,
	&pfPKickingl,
	&pfPKimbalt,
	&pfPKimberle,
	&pfPKingrich,
	&pfPKingrici,
	&pfPKirsty_b,
	&pfPKirstybi,
	&pfPKirsty,
	&pfPKirsty_i,
	&pfPKirstyin,
	&pfPKleptocr,
	&pfPKredit1,
	&pfPLadystar,
	&pfPLarabieb,
	&pfPLarabief,
	&pfPLettera,
	&pfPLetterb,
	&pfPLetterc,
	&pfPLewinsky,
	&pfPLilliput,
	&pfPLivingby,
	&pfPLockergn,
	&pfPLuckyape,
	&pfPLunasol,
	&pfPMailrays,
	&pfPMalache,
	&pfPMapofyou,
	&pfPMarqueem,
	&pfPMetalord,
	&pfPMexcel3d,
	&pfPMexcelle,
	&pfPMinisyst,
	&pfPMinynb,
	&pfPMinynbi,
	&pfPMinyn,
	&pfPMinyni,
	&pfPMisirlod,
	&pfPMisirlou,
	&pfPMobconcr,
	&pfPModelwor,
	&pfPMonofont,
	&pfPMotorcad,
	&pfPMufferaw,
	&pfPNafta,
	&pfPNasaliza,
	&pfPNeurochr,
	&pfPNeuropol,
	&pfPNeurpoli,
	&pfPNewbrill,
	&pfPNumberpi,
	&pfPOctovill,
	&pfPOilcrisa,
	&pfPOilcrisb,
	&pfPOliversb,
	&pfPOrangeki,
	&pfPPakenham,
	&pfPPantspat,
	&pfPPlanetbe,
	&pfPPlasmati,
	&pfPPlasticb,
	&pfPPobeef,
	&pfPPoke,
	&pfPPopup,
	&pfPPresgrg,
	&pfPPricedow,
	&pfPPrima,
	&pfPPrimemin,
	&pfPPrimerb,
	&pfPPrimer,
	&pfPPupcat,
	&pfPQuadapto,
	&pfPQuixotic,
	&pfPRadiohar,
	&pfPRadiosin,
	&pfPRafika,
	&pfPRazorkee,
	&pfPRelishga,
	&pfPRina,
	&pfPRustproo,
	&pfPSandoval,
	&pfPSappm,
	&pfPSavedbyz,
	&pfPScreenge,
	&pfPSexsmith,
	&pfPShifty,
	&pfPShlop,
	&pfPShouldve,
	&pfPShouldvs,
	&pfPSkeletor,
	&pfPSloegin,
	&pfPSofachri,
	&pfPSofachro,
	&pfPSpongy,
	&pfPSquealem,
	&pfPSquealer,
	&pfPSteelfib,
	&pfPSteelfis,
	&pfPSteelout,
	&pfPStereofi,
	&pfPStilltim,
	&pfPStreetcr,
	&pfPStrenu3d,
	&pfPStrenuou,
	&pfPStyrofoa,
	&pfPSubpear,
	&pfPSudbury3,
	&pfPSudbury,
	&pfPSuigb,
	&pfPSuigbi,
	&pfPSuigener,
	&pfPSuigi,
	&pfPSuperglu,
	&pfPSybig,
	&pfPTeenbdit,
	&pfPTeenbold,
	&pfPTeen,
	&pfPTeenital,
	&pfPTeenlita,
	&pfPTeenlite,
	&pfPTerylene,
	&pfPThiamine,
	&pfPTinsnips,
	&pfPTorkb,
	&pfPTorkbi,
	&pfPTork,
	&pfPTorki,
	&pfPTrapperj,
	&pfPTriacsev,
	&pfPTrollbai,
	&pfPTypoderm,
	&pfPUnioncit,
	&pfPUnisb,
	&pfPUnisbi,
	&pfPUnisi,
	&pfPUnispace,
	&pfPUnivox,
	&pfPUnsteady,
	&pfPUrkelian,
	&pfPVademecu,
	&pfPVahikab,
	&pfPVahikac,
	&pfPVahika,
	&pfPVahikai,
	&pfPVanilla,
	&pfPVdub,
	&pfPVectroid,
	&pfPVelvenda,
	&pfPVelvendc,
	&pfPVenusris,
	&pfPVibroceb,
	&pfPVibrocei,
	&pfPVibrocen,
	&pfPVibrocex,
	&pfPVinque,
	&pfPVipnagor,
	&pfPWakebake,
	&pfPWalshes,
	&pfPWelfareb,
	&pfPWildsewe,
	&pfPWintermu,
	&pfPWorldofw,
	&pfPXolto,
	&pfPY2kbug,
	&pfPYonderre,
	&pfPYouregoi,
	&pfPYouregon,
	&pfPYytriumd,
	&pfPZektonbi,
	&pfPZektonbo,
	&pfPZektondo,
	&pfPZekton,
	&pfPZektonit,
	&pfPZeroes,
	&pfPZerohour,
	&pfPZerothre,
	&pfPZerotwos,
	&pfPZodillin,
	&pfPZorque,
	&pfPZrnic,
#endif
	&pfSAstrology,
	&pfSCursive,
	&pfSCyrillic,
	&pfSFutura,
	&pfSFuturaMono,
	&pfSGothicEnglish,
	&pfSGothicGerman,
	&pfSGothicItalian,
	&pfSGreek,
	&pfSJapanese,
	&pfSMarkers,
	&pfSMathLower,
	&pfSMathUpper,
	&pfSMeteorology,
	&pfSMusic,
	&pfSScript,
	&pfSSymbolic,
	&pfSTimesBold,
	&pfSTimesBoldItalic,
	&pfSTimesGreek,
	&pfSTimes,
	&pfSTimesItalic,
	&pfSRoman,
	&pfSRomanMono
};

int pfNumFonts = (sizeof(pfAllFonts) / sizeof(pffont *));

#ifdef __cplusplus
}
#endif

#endif // __POLYFONTS_ALL__
