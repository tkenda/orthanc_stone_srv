/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "../OrthancStone/Sources/Toolbox/GenericToolbox.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <gtest/gtest.h>
#include <stdint.h>
#include <cmath>

#if __cplusplus >= 201103L   // Is C++11?
#  include <cinttypes>  // For PRId64
#else
#  define PRId64 "%lld"
#endif

TEST(GenericToolbox, TestLegitDoubleString)
{
  using OrthancStone::GenericToolbox::LegitDoubleString;

  EXPECT_TRUE(LegitDoubleString("12.34"));
  EXPECT_TRUE(LegitDoubleString("1234"));
  EXPECT_TRUE(LegitDoubleString(".1234"));
  EXPECT_TRUE(LegitDoubleString("1234."));
  EXPECT_TRUE(LegitDoubleString("0.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011234"));
  EXPECT_TRUE(LegitDoubleString("000000000000000000000000000000000000000000000000000000000000000000000000000000000000.00000000000000000000000011234"));
  EXPECT_TRUE(LegitDoubleString("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000112.34"));
  EXPECT_TRUE(LegitDoubleString("000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011234."));
  EXPECT_TRUE(LegitDoubleString("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001123456"));
  EXPECT_TRUE(LegitDoubleString("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001123456000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011234000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011230000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000112345000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011234000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000011234565664565623456"));
  EXPECT_TRUE(LegitDoubleString("1234."));
  EXPECT_TRUE(LegitDoubleString(".0123"));
  EXPECT_TRUE(LegitDoubleString(".123"));
  EXPECT_TRUE(LegitDoubleString(".5"));
  EXPECT_TRUE(LegitDoubleString("."));
  EXPECT_TRUE(LegitDoubleString(""));
  EXPECT_TRUE(LegitDoubleString("0."));
  EXPECT_TRUE(LegitDoubleString(".0"));

  EXPECT_TRUE(LegitDoubleString("1e-15"));
  EXPECT_TRUE(LegitDoubleString("1E-15"));
  EXPECT_TRUE(LegitDoubleString("0.31E-15"));
  EXPECT_TRUE(LegitDoubleString(".0031E-15"));
  EXPECT_TRUE(LegitDoubleString("1e-15"));
  EXPECT_TRUE(LegitDoubleString("1E015"));
  EXPECT_TRUE(LegitDoubleString("0.31E015"));


  EXPECT_FALSE(LegitDoubleString(".5f"));
  EXPECT_FALSE(LegitDoubleString("\n.0031E015"));
  EXPECT_FALSE(LegitDoubleString(".05f"));
  EXPECT_FALSE(LegitDoubleString(" 1 2 "));
  EXPECT_FALSE(LegitDoubleString(" 0.12\t"));
  EXPECT_FALSE(LegitDoubleString(" 0.12"));
  EXPECT_TRUE(LegitDoubleString("0.12\t"));
  EXPECT_TRUE(LegitDoubleString("12\t"));
  EXPECT_FALSE(LegitDoubleString(".01 23"));
  EXPECT_FALSE(LegitDoubleString(". 123"));
  EXPECT_TRUE(LegitDoubleString(".5 "));
  EXPECT_FALSE(LegitDoubleString(" ."));
  EXPECT_FALSE(LegitDoubleString("\n0."));
}

TEST(GenericToolbox, TestLegitIntegerString)
{
  using OrthancStone::GenericToolbox::LegitIntegerString;

  EXPECT_TRUE(LegitIntegerString("1234"));
  EXPECT_TRUE(LegitIntegerString("234"));
  EXPECT_TRUE(LegitIntegerString("01234"));
  EXPECT_TRUE(LegitIntegerString("12340"));
  EXPECT_TRUE(LegitIntegerString("0000000000000011234"));
  EXPECT_TRUE(LegitIntegerString("00000000000000011234"));
  EXPECT_TRUE(LegitIntegerString("00000000000011234"));
  EXPECT_TRUE(LegitIntegerString("112340000000000010"));
  EXPECT_TRUE(LegitIntegerString("0000000000001123456"));
  EXPECT_TRUE(LegitIntegerString("000000000000112345604565665623456"));
  EXPECT_TRUE(LegitIntegerString(""));
  EXPECT_TRUE(LegitIntegerString("0"));
  EXPECT_TRUE(LegitIntegerString("00000"));

  EXPECT_FALSE(LegitIntegerString(".5f"));
  EXPECT_FALSE(LegitIntegerString("1e-15"));
  EXPECT_FALSE(LegitIntegerString("1E-15"));
  EXPECT_FALSE(LegitIntegerString("0.31E-15"));
  EXPECT_FALSE(LegitIntegerString(".0031E-15"));
  EXPECT_FALSE(LegitIntegerString("1e-15"));
  EXPECT_FALSE(LegitIntegerString("1E015"));
  EXPECT_FALSE(LegitIntegerString("0.31E015"));
  EXPECT_FALSE(LegitIntegerString("\n.0031E015"));
  EXPECT_FALSE(LegitIntegerString(".05f"));
  EXPECT_FALSE(LegitIntegerString(" 1 2 "));
  EXPECT_FALSE(LegitIntegerString(" 0.12\t"));
  EXPECT_FALSE(LegitIntegerString(" 0.12"));
  EXPECT_FALSE(LegitIntegerString("0.12\t"));
  EXPECT_FALSE(LegitIntegerString("12\t"));
  EXPECT_FALSE(LegitIntegerString(".01 23"));
  EXPECT_FALSE(LegitIntegerString(". 123"));
  EXPECT_FALSE(LegitIntegerString(".5 "));
  EXPECT_FALSE(LegitIntegerString(" ."));
  EXPECT_FALSE(LegitIntegerString("\n0."));
}


/**
 * The very long "TestStringToDouble" was split in 4 parts. Otherwise,
 * while running in WebAssembly (at least in "Debug" CMAKE_BUILD_TYPE
 * with Emscripten 2.0.0), one get error "failed to asynchronously
 * prepare wasm: CompileError: WebAssembly.instantiate(): Compiling
 * function [...] failed: local count too large". This is because the
 * function is too long.
 **/
TEST(GenericToolbox, TestStringToDouble1)
{
  using OrthancStone::GenericToolbox::StringToDouble;

  const double TOLERANCE = 0.00000000000001;
  double r = 0.0;

  {
    bool ok = StringToDouble(r, "0.0001");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.0001, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.0001");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.0001, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.50217817069333900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.50217817069333900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.96770274105399000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.96770274105399000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.49521088758962000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.49521088758962000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.06201839227379000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.06201839227379000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.33360671999703000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.33360671999703000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.07639304839166000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.07639304839166000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.19287806240687400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.19287806240687400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.44207082838626000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.44207082838626000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.84619708036551800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.84619708036551800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.58091726580509000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.58091726580509000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.18073661859763000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.18073661859763000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.33045549786387000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.33045549786387000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.00272400249168000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.00272400249168000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.95337715877137000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.95337715877137000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "8.95930523708542000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(8.95930523708542000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.78847681371515000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.78847681371515000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.23601540702684000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.23601540702684000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.40676557671367000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.40676557671367000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.36110595246212700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.36110595246212700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.10430292945232000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.10430292945232000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.34892053003478100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.34892053003478100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.86871791690589000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.86871791690589000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.23477571361979100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.23477571361979100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.17723077954105000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.17723077954105000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.55533339430731000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.55533339430731000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.39193581722996000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.39193581722996000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.98290538242799000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.98290538242799000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.39701448187652000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.39701448187652000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.97546141973594000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.97546141973594000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.33870401451186000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.33870401451186000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.15061799435527000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.15061799435527000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.78705704115137000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.78705704115137000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.56210637202493000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.56210637202493000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-8.86139731673717000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-8.86139731673717000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.63169336137189000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.63169336137189000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.93978481744645000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.93978481744645000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.49952444717512000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.49952444717512000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.32659301981935000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.32659301981935000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.59514994228045000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.59514994228045000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.66422938111626000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.66422938111626000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.70431239624531000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.70431239624531000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.22698147029468000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.22698147029468000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.90761005965631200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.90761005965631200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.43368952065867000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.43368952065867000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.79510450171595000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.79510450171595000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.94081596072268000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.94081596072268000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.42019476309409300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.42019476309409300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.70663631642677000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.70663631642677000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.06601188243267550000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.06601188243267550000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.79928310771909400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.79928310771909400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.65577800860582000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.65577800860582000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.62187216187698000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.62187216187698000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.95596656702613300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.95596656702613300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.14349841191783000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.14349841191783000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.23732575725115000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.23732575725115000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.02522229405373000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.02522229405373000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.43364697172459700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.43364697172459700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.39612114240613000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.39612114240613000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.87981321512563200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.87981321512563200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.47459557296809400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.47459557296809400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.10534326849558000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.10534326849558000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.48420825457170000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.48420825457170000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.98994851457562000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.98994851457562000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.18550683277018200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.18550683277018200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.79951199056989300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.79951199056989300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.92573951347502000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.92573951347502000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.46138476058529000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.46138476058529000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.34518431607109000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.34518431607109000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.33372656820168000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.33372656820168000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.16931283159188600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.16931283159188600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.97223922802124000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.97223922802124000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.48394627491386000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.48394627491386000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.88861737945960600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.88861737945960600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.85676190081840000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.85676190081840000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.54459170417494000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.54459170417494000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.16447870264995300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.16447870264995300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.35795535411029000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.35795535411029000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.29431172135530300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.29431172135530300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.96558311276619000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.96558311276619000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.81681460880669000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.81681460880669000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.20509941503951000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.20509941503951000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.72765905661257000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.72765905661257000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.48788237089759900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.48788237089759900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.24947907141902000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.24947907141902000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.59005387432649000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.59005387432649000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.30370570926522000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.30370570926522000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.73638792046556000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.73638792046556000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.87789934199453800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.87789934199453800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.51989255137937000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.51989255137937000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.76305470679095000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.76305470679095000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.86920962997342000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.86920962997342000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.91313411328065000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.91313411328065000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.73463683758381000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.73463683758381000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.84273889473222500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.84273889473222500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.87403925546477700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.87403925546477700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.36964126011414000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.36964126011414000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.02726746648694000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.02726746648694000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.50557053097483000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.50557053097483000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.56453106035648000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.56453106035648000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.61890516636808000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.61890516636808000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.37767835277405000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.37767835277405000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.90511255527429100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.90511255527429100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.05929345122920000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.05929345122920000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.21311454144036000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.21311454144036000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-7.79062987304713000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-7.79062987304713000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.21365525338096000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.21365525338096000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.28348152906416000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.28348152906416000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.06610409505261000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.06610409505261000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.35302095923550200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.35302095923550200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.90818370281786000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.90818370281786000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.32125632829404000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.32125632829404000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.19461589112926800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.19461589112926800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.13206147532649300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.13206147532649300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.90445975568758000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.90445975568758000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.09055301456874000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.09055301456874000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.94747584830211900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.94747584830211900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.87479371073786000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.87479371073786000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.77693922561847000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.77693922561847000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.43857452366099000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.43857452366099000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.32571155407419000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.32571155407419000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.02598140411007480000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.02598140411007480000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.63213858956142000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.63213858956142000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.87199046737281000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.87199046737281000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.51485641768478000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.51485641768478000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.64286402800302700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.64286402800302700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.47677130142230000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.47677130142230000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.39498987162520000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.39498987162520000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.97846593865349600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.97846593865349600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.38696988049949000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.38696988049949000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.99716557343840900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.99716557343840900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.26983285318203300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.26983285318203300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.02818282704670500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.02818282704670500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.33995460770471000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.33995460770471000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.90961343273142000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.90961343273142000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.70545858631691000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.70545858631691000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.99837322296447000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.99837322296447000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.52931499785106000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.52931499785106000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.50600351005455000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.50600351005455000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.83191012798055900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.83191012798055900000, r, TOLERANCE);
  }
}

TEST(GenericToolbox, TestStringToDouble2)
{
  using OrthancStone::GenericToolbox::StringToDouble;

  const double TOLERANCE = 0.00000000000001;
  double r = 0.0;

  {
    bool ok = StringToDouble(r, "2.58090819604341000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.58090819604341000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.95182376827953000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.95182376827953000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.04199841193785000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.04199841193785000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.17938850513021000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.17938850513021000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.66797071567664000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.66797071567664000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.37221015583147000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.37221015583147000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.75673862000485000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.75673862000485000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.79003986824116500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.79003986824116500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.86020949016507000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.86020949016507000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.14082258481500000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.14082258481500000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.71685664840859000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.71685664840859000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.93998389083824300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.93998389083824300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.77244357996158000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.77244357996158000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.10595524850565900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.10595524850565900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.69799635213612000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.69799635213612000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.57971250175452400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.57971250175452400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.92766866933807100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.92766866933807100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.46991620588858000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.46991620588858000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.94569644123488000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.94569644123488000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.18859094010287000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.18859094010287000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.03213167005865000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.03213167005865000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "8.81754146434609000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(8.81754146434609000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.75897430327076600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.75897430327076600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.80047028975912000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.80047028975912000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.00529573224131364000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.00529573224131364000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.71024073322357000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.71024073322357000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.60642130185119000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.60642130185119000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.09793780927960000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.09793780927960000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.18560965637846000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.18560965637846000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.13078526893487000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.13078526893487000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.19951899215254000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.19951899215254000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.81885534502479000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.81885534502479000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.00480638980341000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.00480638980341000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.35315675289406200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.35315675289406200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.29812812014442000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.29812812014442000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.98878626408816000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.98878626408816000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.34644737073484000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.34644737073484000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.37478492823657000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.37478492823657000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.97205178784195000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.97205178784195000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.65165003646427000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.65165003646427000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.89236175545723000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.89236175545723000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.80366872242454000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.80366872242454000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "7.65465855719486000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(7.65465855719486000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.51455943741659600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.51455943741659600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.14337541345649000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.14337541345649000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.06909574569091000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.06909574569091000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.07698497525470000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.07698497525470000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.04223854975535000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.04223854975535000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.46422724459484000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.46422724459484000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.65888981424971000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.65888981424971000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-7.10193673069906000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-7.10193673069906000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.77638222509466500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.77638222509466500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.15543610545042000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.15543610545042000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.51787760900314000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.51787760900314000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.09022915694655000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.09022915694655000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.41861013154040000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.41861013154040000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.40227565288403000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.40227565288403000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.44321592617247400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.44321592617247400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.34090258417639000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.34090258417639000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.54291265629528700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.54291265629528700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.70700051509186000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.70700051509186000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-6.55072864947955000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-6.55072864947955000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.96741942560520000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.96741942560520000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.55202552301084000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.55202552301084000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.36133250863907300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.36133250863907300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.46513564511238000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.46513564511238000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.97424909475891000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.97424909475891000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.87005014400085000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.87005014400085000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.25552308785543000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.25552308785543000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.43365620710902500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.43365620710902500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.17392137573999000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.17392137573999000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.56870774575795000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.56870774575795000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.07449225479459900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.07449225479459900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.25905472211571000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.25905472211571000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.13708454690765000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.13708454690765000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.08223808231444500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.08223808231444500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.69624060459529000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.69624060459529000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.87232652840742000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.87232652840742000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.20739068103174300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.20739068103174300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.45449313279700600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.45449313279700600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.06604828436047000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.06604828436047000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.16603807756896700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.16603807756896700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "6.56288534361719000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(6.56288534361719000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.28481655900710000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.28481655900710000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.79412040010646300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.79412040010646300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.90088144503330000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.90088144503330000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.65278657648370200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.65278657648370200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.40305895338068000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.40305895338068000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.07193308249503000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.07193308249503000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.83752112822253600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.83752112822253600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.63174453257058400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.63174453257058400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.80163760021425000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.80163760021425000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.57922670044433000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.57922670044433000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "6.80309348037215000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(6.80309348037215000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.03658264005365000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.03658264005365000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "8.57714214650747000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(8.57714214650747000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.25657256359494300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.25657256359494300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.07218601388076000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.07218601388076000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.70300607815345600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.70300607815345600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.06822028770915030000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.06822028770915030000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.52253514473857300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.52253514473857300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.89211508282910000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.89211508282910000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.47331243043688000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.47331243043688000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.77190031720697000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.77190031720697000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.80704979593058400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.80704979593058400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.58398766715845000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.58398766715845000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.59532008540482000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.59532008540482000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.92824570343456000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.92824570343456000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.15232705272560400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.15232705272560400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.13670276871382500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.13670276871382500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.20063314286385000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.20063314286385000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.20390958339690000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.20390958339690000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.01999231401200000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.01999231401200000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.33696129476675000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.33696129476675000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.97472839619216000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.97472839619216000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.25935508044004000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.25935508044004000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.98737992668548000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.98737992668548000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.12647380973595000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.12647380973595000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.04573005673487000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.04573005673487000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.40131707240240000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.40131707240240000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.65350895248975000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.65350895248975000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.94344081509933000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.94344081509933000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.72697189247371000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.72697189247371000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-6.67990308483490000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-6.67990308483490000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.32343310660542000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.32343310660542000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.78517123090950000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.78517123090950000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.25849816293583000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.25849816293583000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.75396267700095000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.75396267700095000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.07647901824168000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.07647901824168000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.38047538070258000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.38047538070258000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.20758597742145100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.20758597742145100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.85537090667122100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.85537090667122100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.76805423797310000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.76805423797310000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.40449492713592000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.40449492713592000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.62167096457336000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.62167096457336000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.74002997550002000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.74002997550002000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.42443064164790400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.42443064164790400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.27951604455776900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.27951604455776900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.51579267322296100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.51579267322296100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.36457251883339000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.36457251883339000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.24583724281163800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.24583724281163800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.89377268220461400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.89377268220461400000, r, TOLERANCE);
  }
}

TEST(GenericToolbox, TestStringToDouble3)
{
  using OrthancStone::GenericToolbox::StringToDouble;

  const double TOLERANCE = 0.00000000000001;
  double r = 0.0;

  {
    bool ok = StringToDouble(r, "1.45674815825147000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.45674815825147000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.85885778179785000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.85885778179785000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.46665640857091000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.46665640857091000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.20955012166670000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.20955012166670000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.56901773371710000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.56901773371710000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.28236715260714000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.28236715260714000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.68701183150938000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.68701183150938000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.52491544332882000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.52491544332882000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.35369978756681100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.35369978756681100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.37511760913818000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.37511760913818000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.97143364160106000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.97143364160106000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.24559477959438200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.24559477959438200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.75423032204965000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.75423032204965000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.32370293533555300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.32370293533555300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.91057697616735300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.91057697616735300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.47061739750017000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.47061739750017000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.00584944044255000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.00584944044255000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.50109276836214000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.50109276836214000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.55007311077336000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.55007311077336000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "6.72362848947278000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(6.72362848947278000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.01151577930873910000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.01151577930873910000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.42911860719965600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.42911860719965600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.66111289816664900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.66111289816664900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.86619326895662000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.86619326895662000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.55732089555551800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.55732089555551800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.30341160871063000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.30341160871063000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.56416171751671000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.56416171751671000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.18594183907073900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.18594183907073900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.76842629255481000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.76842629255481000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.51401910241563500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.51401910241563500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.22475819701855600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.22475819701855600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.52647532265208000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.52647532265208000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.36302691626541400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.36302691626541400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.97344494357431000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.97344494357431000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.55983273528683000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.55983273528683000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.11831213859734000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.11831213859734000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.65912510665320000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.65912510665320000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.49382686162217300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.49382686162217300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.82681319206813000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.82681319206813000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.63990018376158400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.63990018376158400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.46190583889476000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.46190583889476000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.33778970852365000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.33778970852365000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.67479071577411000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.67479071577411000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.92524843393689500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.92524843393689500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.25880026429762000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.25880026429762000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.74489327613996700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.74489327613996700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.81221138965657700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.81221138965657700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.63922583575742000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.63922583575742000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.46277795175279000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.46277795175279000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.92701639727950000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.92701639727950000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.00608886511047000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.00608886511047000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.59692755566202000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.59692755566202000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.43660191582482000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.43660191582482000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.81340386111566000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.81340386111566000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.41381029424169000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.41381029424169000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.19067619994638000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.19067619994638000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.41344288416300500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.41344288416300500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.19449050806631000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.19449050806631000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.94346623486537000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.94346623486537000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.15222182306952000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.15222182306952000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.16597270635016000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.16597270635016000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.70800933434033500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.70800933434033500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.01520859362049000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.01520859362049000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.99808924291921000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.99808924291921000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.46413571523617000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.46413571523617000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.23372155013436100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.23372155013436100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.22220872747082200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.22220872747082200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.45231083327185000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.45231083327185000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.18629931302726700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.18629931302726700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.25902351261081000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.25902351261081000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.74979626491734000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.74979626491734000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.96938763187002300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.96938763187002300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.01957662295404000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.01957662295404000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.29052978268713000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.29052978268713000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.72223107008226000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.72223107008226000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.02075269473024000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.02075269473024000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.41254866425811000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.41254866425811000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.79485280000328000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.79485280000328000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.71346724218879000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.71346724218879000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.02769972220451300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.02769972220451300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.30840233811538300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.30840233811538300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.46998368658050000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.46998368658050000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.39027116095637000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.39027116095637000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.76287623175477000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.76287623175477000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.32254147772188000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.32254147772188000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.43476530791568300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.43476530791568300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.15293149279800000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.15293149279800000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.52187680632247000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.52187680632247000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.81464816227136000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.81464816227136000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.45410471462063000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.45410471462063000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.05770661428355000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.05770661428355000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.13365631051443000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.13365631051443000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.78752268413674000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.78752268413674000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.07653691039301000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.07653691039301000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.69590678743817200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.69590678743817200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.16750017237716000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.16750017237716000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.80454059859949500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.80454059859949500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.01080121519000000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.01080121519000000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-6.26823154211325000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-6.26823154211325000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.27168923945051000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.27168923945051000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.95882006177823000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.95882006177823000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.29782169884960000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.29782169884960000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.18868107998160000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.18868107998160000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.42221680213317000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.42221680213317000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.97658929351465000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.97658929351465000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.76786358453912000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.76786358453912000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.63996015852897000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.63996015852897000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.53048948235281000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.53048948235281000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.72713707173900000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.72713707173900000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.67678586641071000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.67678586641071000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.27938145860632000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.27938145860632000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.67198854485259000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.67198854485259000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.08448300379640000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.08448300379640000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.89200760812645600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.89200760812645600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.84610740591283000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.84610740591283000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.33520422865196000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.33520422865196000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.26977509689943300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.26977509689943300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.06556998317024000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.06556998317024000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.10187258099846900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.10187258099846900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.17925123727943000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.17925123727943000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.53744857107300000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.53744857107300000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.79170718052687000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.79170718052687000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.70094405912437000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.70094405912437000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.36090079790873000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.36090079790873000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.24214402582849600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.24214402582849600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.61857148054390100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.61857148054390100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.49681404951875000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.49681404951875000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.62901170744691000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.62901170744691000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.31812686057237000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.31812686057237000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.29232513324991000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.29232513324991000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.30415968616239000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.30415968616239000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.23085063327904200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.23085063327904200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.55328286749515200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.55328286749515200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.85987085857330000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.85987085857330000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.91580898949892000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.91580898949892000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.83451772893723400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.83451772893723400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "8.47663066417390000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(8.47663066417390000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.07750241770625000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.07750241770625000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.79888627452876900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.79888627452876900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.62390154942094000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.62390154942094000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.15344123017231000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.15344123017231000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.29946850732165400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.29946850732165400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.43195118421230900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.43195118421230900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.96541584823575200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.96541584823575200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.31046639376194000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.31046639376194000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.80868720295308000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.80868720295308000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.91875650345864000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.91875650345864000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.16383120358956700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.16383120358956700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.70602187714556100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.70602187714556100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.59908461641224000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.59908461641224000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.75777826959967000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.75777826959967000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "7.51297060665513000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(7.51297060665513000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.26428182282563100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.26428182282563100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.39790664337099000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.39790664337099000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.52727246472497000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.52727246472497000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.15622706860781000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.15622706860781000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.33838258813926000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.33838258813926000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.68209356689853000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.68209356689853000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.12950059731897000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.12950059731897000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.24366467557925000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.24366467557925000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.49620375847259000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.49620375847259000000, r, TOLERANCE);
  }
}

TEST(GenericToolbox, TestStringToDouble4)
{
  using OrthancStone::GenericToolbox::StringToDouble;

  const double TOLERANCE = 0.00000000000001;
  double r = 0.0;

  {
    bool ok = StringToDouble(r, "-2.04336416841016000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.04336416841016000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.67258592218424000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.67258592218424000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.50983053528049600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.50983053528049600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.91671084717300400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.91671084717300400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-8.44023177630015000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-8.44023177630015000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.74048232685721000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.74048232685721000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.26893036021697000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.26893036021697000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.81851986861265000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.81851986861265000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.15033199581975000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.15033199581975000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.78498201393837000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.78498201393837000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.05287486584367510000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.05287486584367510000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.61135813076181000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.61135813076181000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.28026567889772000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.28026567889772000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.42191037602383000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.42191037602383000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.91926628714024000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.91926628714024000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.13695172353534000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.13695172353534000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.19234124167404000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.19234124167404000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.54749310279860000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.54749310279860000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.79683457995789900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.79683457995789900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.35976469553121900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.35976469553121900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.77036485893720200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.77036485893720200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.05245602278075000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.05245602278075000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.82693752156961000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.82693752156961000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.59176692084240000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.59176692084240000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.59390017044970000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.59390017044970000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.33597209441560000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.33597209441560000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.73223852215944000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.73223852215944000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.36562951036666300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.36562951036666300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.16083819415565000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.16083819415565000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.19457461912900000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.19457461912900000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.48993781857833000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.48993781857833000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.91089514047878000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.91089514047878000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.67713158365996000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.67713158365996000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.14929866451844800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.14929866451844800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.76110653286820000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.76110653286820000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.05937778946509720000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.05937778946509720000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.67737188304973000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.67737188304973000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.58425440578219000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.58425440578219000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.30491550374261000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.30491550374261000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.48379945880357000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.48379945880357000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.62987027701035800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.62987027701035800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.97181285150671000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.97181285150671000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.24881707556359700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.24881707556359700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.83925282156180000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.83925282156180000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.07567311295324000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.07567311295324000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.78919598870022000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.78919598870022000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.51240908161798000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.51240908161798000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.51275535534832000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.51275535534832000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.58920518726789000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.58920518726789000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.81944139206950000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.81944139206950000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.20584689863215000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.20584689863215000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.52387820278697400000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.52387820278697400000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.41030960929320000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.41030960929320000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.92103214885374000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.92103214885374000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.24231540245246000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.24231540245246000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.16908726665941100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.16908726665941100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.44589887686242000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.44589887686242000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.49069006371512000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.49069006371512000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.80650544937931100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.80650544937931100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.97976397104165000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.97976397104165000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.98766812819005000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.98766812819005000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.72989593663204100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.72989593663204100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.61947487048298000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.61947487048298000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.14305085135282200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.14305085135282200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.19625994631193500000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.19625994631193500000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.37918205114016000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.37918205114016000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "6.59599108171267000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(6.59599108171267000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.96455017519345000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.96455017519345000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.54659210340770000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.54659210340770000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.34333955584988000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.34333955584988000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.97945807230247000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.97945807230247000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.16309656911088000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.16309656911088000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.25046325751587000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.25046325751587000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.95513333613538000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.95513333613538000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.44180657712571200000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.44180657712571200000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.62068238736436000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.62068238736436000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.06550714914445000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.06550714914445000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.50821128944561000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.50821128944561000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.15508838007900000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.15508838007900000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.95233817795899000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.95233817795899000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.51496658163574000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.51496658163574000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.78333801715048000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.78333801715048000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.21314186040171000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.21314186040171000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.66527690284710800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.66527690284710800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.15441313415350000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.15441313415350000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.23491685110319000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.23491685110319000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.72724695951577000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.72724695951577000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.24050455306641300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.24050455306641300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.21656863480457000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.21656863480457000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.26488830552906000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.26488830552906000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "3.75588617365038000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(3.75588617365038000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.03323480544193850000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.03323480544193850000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.09120742547457650000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.09120742547457650000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-7.88263056036503000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-7.88263056036503000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.43816026309627000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.43816026309627000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-7.03193105607121000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-7.03193105607121000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.60611554369909000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.60611554369909000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-5.51585989717609000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-5.51585989717609000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.07820571638609000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.07820571638609000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.06101375865811000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.06101375865811000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.20736962161768000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.20736962161768000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.90243061828996000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.90243061828996000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.85299495975262000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.85299495975262000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.12934888152265000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.12934888152265000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.67072919212958000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.67072919212958000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.83114509924264900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.83114509924264900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.30250616100438000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.30250616100438000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.12093048302870000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.12093048302870000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.05552960660102000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.05552960660102000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.49292325032676000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.49292325032676000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.17400757029104000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.17400757029104000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.14267109660887000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.14267109660887000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.10546669054034000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.10546669054034000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.21371952871041700000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.21371952871041700000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "4.78156583044177000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(4.78156583044177000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.50472792044367000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.50472792044367000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.12605755507866600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.12605755507866600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.70371185139311000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.70371185139311000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.10053982101354000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.10053982101354000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.83624586947925000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.83624586947925000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.05046060224221000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.05046060224221000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.28157147555257100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.28157147555257100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.59637285322805000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.59637285322805000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.75470175557419100000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.75470175557419100000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.70838399472621000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.70838399472621000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.45654131621183000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.45654131621183000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.28443945581399000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.28443945581399000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.68823597183684000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.68823597183684000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.29650435341174000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.29650435341174000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.90134290476188000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.90134290476188000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.18487205108194000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.18487205108194000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.14778330708372000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.14778330708372000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.98574838531856000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.98574838531856000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.38116626593387000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.38116626593387000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.18109367323846900000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.18109367323846900000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.54919024558896000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.54919024558896000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-3.01819062231017000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-3.01819062231017000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.86141885135950000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.86141885135950000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.31984756442573800000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.31984756442573800000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.35256585949514000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.35256585949514000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-6.04254591090669000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-6.04254591090669000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.31151799331342300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.31151799331342300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.77556498660193000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.77556498660193000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "5.90371566906766000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(5.90371566906766000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.29825016398122000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.29825016398122000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.01456323654512000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.01456323654512000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-6.19305288625244000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-6.19305288625244000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.99509367627092600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.99509367627092600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.08519786419394440000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.08519786419394440000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.88317752752055000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.88317752752055000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "1.69592260047492000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(1.69592260047492000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.66260089028084000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.66260089028084000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.12882625389413000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.12882625389413000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.79536921500302000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.79536921500302000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-4.51399167357593000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-4.51399167357593000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-0.75817764527332300000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-0.75817764527332300000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-2.12821371262498000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-2.12821371262498000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "-1.08153732327358000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(-1.08153732327358000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "0.71608571781169600000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(0.71608571781169600000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.42004689052701000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.42004689052701000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.84542164846610000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.84542164846610000000, r, TOLERANCE);
  }

  {
    bool ok = StringToDouble(r, "2.97822513569917000000");
    EXPECT_TRUE(ok);
    EXPECT_NEAR(2.97822513569917000000, r, TOLERANCE);
  }
}

TEST(GenericToolbox, TestStringToDoubleHard)
{
  using OrthancStone::GenericToolbox::StringToDouble;
  const double TOLERANCE = 0.00000000000001;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;
  for (double b = DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {
    char txt[1024];
#if defined(_MSC_VER)
    sprintf_s(txt, "%.17f", b);
#else
    snprintf(txt, sizeof(txt) - 1, "%.17f", b);
#endif
    double r = 0.0;
    bool ok = StringToDouble(r, txt);
    
#if 0
    if (ok)
    {
      printf("OK for txt = \"%s\" and r = %.17f\n", txt, r);
    }
    else
    {
      printf("Not ok for txt = \"%s\" and r = %.17f\n", txt, r);
      ok = StringToDouble(r, txt);
    }
#endif

    EXPECT_TRUE(ok);

#if 0
    if (fabs(b - r) > TOLERANCE)
    {
      printf("fabs(b (%.17f) - r (%.17f)) ((%.17f))  > TOLERANCE (%.17f)\n", b, r, fabs(b-r), TOLERANCE);
    }
#endif
    EXPECT_NEAR(b, r, TOLERANCE);
  }
}

TEST(GenericToolbox, TestStringToDoubleHardNeg)
{
  using OrthancStone::GenericToolbox::StringToDouble;
  const double TOLERANCE = 0.00000000000001;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;
  for (double b = -1.0*DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {
    char txt[1024];
#if defined(_MSC_VER)
    sprintf_s(txt, "%.17f", b);
#else
    snprintf(txt, sizeof(txt) - 1, "%.17f", b);
#endif
    double r = 0.0;
    bool ok = StringToDouble(r, txt);

#if 0
    if (ok)
    {
      printf("OK for txt = \"%s\" and r = %.17f\n", txt, r);
    }
    else
    {
      printf("Not ok for txt = \"%s\" and r = %.17f\n", txt, r);
      ok = StringToDouble(r, txt);
    }
#endif

    EXPECT_TRUE(ok);

#if 0
    if (fabs(b - r) > TOLERANCE)
    {
      printf("fabs(b (%.17f) - r (%.17f)) ((%.17f))  > TOLERANCE (%.17f)\n", b, r, fabs(b - r), TOLERANCE);
    }
#endif
    EXPECT_NEAR(b, r, TOLERANCE);
  }
}

static const size_t NUM_TIMINGS_CONVS = 1; // set to 2000 if you want to measure perfs;


//4444444444444444$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

TEST(GenericToolbox, TestStringToDoubleHardNeg_lexical_cast_vs_StringToDouble)
{
  using OrthancStone::GenericToolbox::StringToDouble;
  const double TOLERANCE = 0.00000000000001;

  double total_us_StringToDouble = 0.0;
  double total_us_lexical_cast = 0.0;
  int64_t numConversions = 0;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;
  for (double b = -1.0 * DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {
    char txt[1024];
#if defined(_MSC_VER)
    sprintf_s(txt, "%.17f", b);
#else
    snprintf(txt, sizeof(txt) - 1, "%.17f", b);
#endif
    
    
    double r = 0.0;

    bool ok = true;

    {
      boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
      for (size_t i = 0; i < NUM_TIMINGS_CONVS; ++i)
      {
        ok = StringToDouble(r, txt);
      }
      boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
      total_us_StringToDouble += (end - start).total_microseconds();
    }

    {
      boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
      for (size_t i = 0; i < NUM_TIMINGS_CONVS; ++i)
      {
        try
        {
          r = boost::lexical_cast<double>(txt);
          ok = true;
        }
        catch (boost::bad_lexical_cast& )
        {
          ok = false;
        }
      }
      boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
      total_us_lexical_cast += (end - start).total_microseconds();
    }
    numConversions += NUM_TIMINGS_CONVS;

#if 0
    if (ok)
    {
      printf("OK for txt = \"%s\" and r = %.17f\n", txt, r);
    }
    else
    {
      printf("Not ok for txt = \"%s\" and r = %.17f\n", txt, r);
      ok = StringToDouble(r, txt);
    }
#endif

    EXPECT_TRUE(ok);

#if 0
    if (fabs(b - r) > TOLERANCE)
    {
      printf("fabs(b (%.17f) - r (%.17f)) ((%.17f))  > TOLERANCE (%.17f)\n", b, r, fabs(b - r), TOLERANCE);
    }
#endif
    EXPECT_NEAR(b, r, TOLERANCE);
  }
  std::cout << "Total time (us) for " << numConversions 
    << " conversions using StringToDouble (with NO scientific notation)      = " 
    << static_cast<int64_t>(total_us_StringToDouble) << std::endl;
  
  std::cout << "Time per conversion using StringToDouble (ns) = " 
    << (int64_t)( (total_us_StringToDouble * 1000) /((double)numConversions)) << std::endl;

  std::cout << "Total time (us) for " << numConversions 
    << " conversions using boost::lexical_cast (with NO scientific notation) = " 
    << static_cast<int64_t>(total_us_lexical_cast) << std::endl;

  std::cout << "Time per conversion using boost::lexical_cast (ns) = "
    << (int64_t)( (total_us_lexical_cast * 1000) / ((double)numConversions)) << std::endl;

  std::cout << "StringToDouble is " << (int)((total_us_lexical_cast / total_us_StringToDouble) + 0.5) << " times faster than boost::lexical_cast" << std::endl;

}
//4444444444444444$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


TEST(GenericToolbox, TestStringToDoubleHardScientific)
{
  using OrthancStone::GenericToolbox::StringToDouble;
  const double TOLERANCE = 0.00000000000001;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;
  for (double b = DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {

    // the tolerance must be adapted depending on the exponent
    double exponent = (b == 0) ? 0 : 1.0 + std::floor(std::log10(std::fabs(b)));
    double actualTolerance = TOLERANCE * pow(10.0, exponent);

    char txt[1024];
#if defined(_MSC_VER)
    sprintf_s(txt, "%.17e", b);
#else
    snprintf(txt, sizeof(txt) - 1, "%.17e", b);
#endif
    double r = 0.0;
    bool ok = StringToDouble(r, txt);

#if 0
    if (ok)
    {
      printf("OK for txt = \"%s\" and r = %.17e\n", txt, r);
    }
    else
    {
      printf("Not ok for txt = \"%s\" and r = %.17e\n", txt, r);
      ok = StringToDouble(r, txt);
    }
#endif

    EXPECT_TRUE(ok);

#if 0
    if (fabs(b - r) > actualTolerance)
    {
      printf("NOK fabs(b (%.17f) - r (%.17f)) ((%.17f))  > actualTolerance (%.17f)\n", b, r, fabs(b - r), actualTolerance);
      printf("NOK fabs(b (%.17e) - r (%.17e)) ((%.17e))  > actualTolerance (%.17e)\n", b, r, fabs(b - r), actualTolerance);
      ok = StringToDouble(r, txt);
    }
    else
    {
      printf("OK  fabs(b (%.17f) - r (%.17f)) ((%.17f)) <= actualTolerance (%.17f)\n", b, r, fabs(b - r), actualTolerance);
      printf("OK  fabs(b (%.17e) - r (%.17e)) ((%.17e)) <= actualTolerance (%.17e)\n", b, r, fabs(b - r), actualTolerance);
    }
#endif
    EXPECT_NEAR(b, r, actualTolerance);
  }
}

TEST(GenericToolbox, TestStringToDoubleHardNegScientific)
{
  using OrthancStone::GenericToolbox::StringToDouble;
  const double TOLERANCE = 0.00000000000001;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;
  for (double b = -1.0 * DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {
    // the tolerance must be adapted depending on the exponent
    double exponent = (b == 0) ? 0 : 1.0 + std::floor(std::log10(std::fabs(b)));
    double actualTolerance = TOLERANCE * pow(10.0, exponent);

    char txt[1024];
#if defined(_MSC_VER)
    sprintf_s(txt, "%.17e", b);
#else
    snprintf(txt, sizeof(txt) - 1, "%.17e", b);
#endif
    double r = 0.0;
    bool ok = StringToDouble(r, txt);

#if 0
    if (ok)
    {
      printf("OK for txt = \"%s\" and r = %.17e\n", txt, r);
    }
    else
    {
      printf("Not ok for txt = \"%s\" and r = %.17e\n", txt, r);
      ok = StringToDouble(r, txt);
    }
#endif

    EXPECT_TRUE(ok);

#if 0
    if (fabs(b - r) > actualTolerance)
    {
      printf("NOK fabs(b (%.17f) - r (%.17f)) ((%.17f))  > actualTolerance (%.17f)\n", b, r, fabs(b - r), actualTolerance);
      printf("NOK fabs(b (%.17e) - r (%.17e)) ((%.17e))  > actualTolerance (%.17e)\n", b, r, fabs(b - r), actualTolerance);
      ok = StringToDouble(r, txt);
    }
    else
    {
      printf("OK  fabs(b (%.17f) - r (%.17f)) ((%.17f)) <= actualTolerance (%.17f)\n", b, r, fabs(b - r), actualTolerance);
      printf("OK  fabs(b (%.17e) - r (%.17e)) ((%.17e)) <= actualTolerance (%.17e)\n", b, r, fabs(b - r), actualTolerance);
    }
#endif
    EXPECT_NEAR(b, r, actualTolerance);
  }
}


TEST(GenericToolbox, TestStringToDoubleHardNegScientific_lexical_cast_vs_StringToDouble)
{
  using OrthancStone::GenericToolbox::StringToDouble;
  const double TOLERANCE = 0.00000000000001;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;

  double total_us_StringToDouble = 0.0;
  double total_us_lexical_cast = 0.0;
  int64_t numConversions = 0;

  for (double b = -1.0 * DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {
    // the tolerance must be adapted depending on the exponent
    double exponent = (b == 0) ? 0 : 1.0 + std::floor(std::log10(std::fabs(b)));
    double actualTolerance = TOLERANCE * pow(10.0, exponent);

    char txt[1024];
#if defined(_MSC_VER)
    sprintf_s(txt, "%.17e", b);
#else
    snprintf(txt, sizeof(txt) - 1, "%.17e", b);
#endif
    double r = 0.0;

    bool ok = true;

    {
      boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
      for (size_t i = 0; i < NUM_TIMINGS_CONVS; ++i)
      {
        ok = StringToDouble(r, txt);
      }
      boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
      total_us_StringToDouble += (end - start).total_microseconds();
    }

    {
      boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
      for (size_t i = 0; i < NUM_TIMINGS_CONVS; ++i)
      {
        try
        {
          r = boost::lexical_cast<double>(txt);
          ok = true;
        }
        catch (boost::bad_lexical_cast& )
        {
          ok = false;
        }
      }
      boost::posix_time::ptime end = boost::posix_time::microsec_clock::local_time();
      total_us_lexical_cast += (end - start).total_microseconds();
    }
    numConversions += NUM_TIMINGS_CONVS;

#if 0
    if (ok)
    {
      printf("OK for txt = \"%s\" and r = %.17e\n", txt, r);
    }
    else
    {
      printf("Not ok for txt = \"%s\" and r = %.17e\n", txt, r);
      ok = StringToDouble(r, txt);
    }
#endif

    EXPECT_TRUE(ok);

#if 0
    if (fabs(b - r) > actualTolerance)
    {
      printf("NOK fabs(b (%.17f) - r (%.17f)) ((%.17f))  > actualTolerance (%.17f)\n", b, r, fabs(b - r), actualTolerance);
      printf("NOK fabs(b (%.17e) - r (%.17e)) ((%.17e))  > actualTolerance (%.17e)\n", b, r, fabs(b - r), actualTolerance);
      ok = StringToDouble(r, txt);
    }
    else
    {
      printf("OK  fabs(b (%.17f) - r (%.17f)) ((%.17f)) <= actualTolerance (%.17f)\n", b, r, fabs(b - r), actualTolerance);
      printf("OK  fabs(b (%.17e) - r (%.17e)) ((%.17e)) <= actualTolerance (%.17e)\n", b, r, fabs(b - r), actualTolerance);
    }
#endif
    EXPECT_NEAR(b, r, actualTolerance);
  }

  std::cout << "Total time (us) for " << numConversions
    << " conversions using StringToDouble (WITH scientific notation)      = "
    << static_cast<int64_t>(total_us_StringToDouble) << std::endl;

  std::cout << "Time per conversion using StringToDouble (ns) = "
    << (int64_t)( (total_us_StringToDouble*1000) / ((double)numConversions)) << std::endl;

  std::cout << "Total time (us) for " << numConversions
    << " conversions using boost::lexical_cast (WITH  scientific notation) = "
    << static_cast<int64_t>(total_us_lexical_cast) << std::endl;

  std::cout << "Time per conversion using boost::lexical_cast (ns) = "
    << (int64_t)( (total_us_lexical_cast * 1000) / ((double)numConversions)) << std::endl;

  std::cout << "StringToDouble is " << (int)((total_us_lexical_cast / total_us_StringToDouble)+ 0.5) << " times faster than boost::lexical_cast" << std::endl;
}


TEST(GenericToolbox, TestStringToIntegerHard)
{
  using OrthancStone::GenericToolbox::StringToInteger;

  size_t i = 0;
  const size_t COUNT = 125;
  //const double FACTOR = 1.000000000171271211;
  const double FACTOR = 1.71271211;
  for (double b = DBL_EPSILON; b < DBL_MAX && i < COUNT; ++i, b *= FACTOR)
  {
    int64_t bi = static_cast<int64_t>(b);

    char txt[1024];
#if defined(_MSC_VER)
#  if (_MSC_VER > 1800)
    sprintf_s(txt, "%lld", bi);
#  else
    sprintf_s(txt, "%I64d", bi);
#  endif
#else
    snprintf(txt, sizeof(txt) - 1, "%" PRId64, bi);  // https://stackoverflow.com/a/9225648/881731
#endif
    
    int64_t r = 0;
    bool ok = StringToInteger<int64_t>(r, txt);
    EXPECT_TRUE(ok);
    EXPECT_EQ(bi, r);
#if 0
    if (ok)
    {
      printf("OK for b = %.17f bi = %" PRId64 " txt = \"%s\" and r = %" PRId64 "\n", b, bi, txt, r);
    }
    else
    {
      printf("NOK for b = %.17f bi = %" PRId64 " txt = \"%s\" and r = %" PRId64 "\n", b, bi, txt, r);
      ok = StringToInteger<int64_t>(r, txt);
    }
#endif
  }
}


TEST(GenericToolbox, TestGetRgbValuesFromString)
{
  using OrthancStone::GenericToolbox::GetRgbValuesFromString;

  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;

  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, ""));
  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, " "));
  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, "rgb() "));
  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, "rgb(12,30 2563) "));
  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, "rgb(12,30 2563,45) "));
  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, "rgb(12,303.23,45)"));
  EXPECT_FALSE(GetRgbValuesFromString(red, green, blue, "rgb(12,303,45 "));

  ASSERT_TRUE(GetRgbValuesFromString(red, green, blue, "rgb(12,255,45)"));
  EXPECT_EQ(12, red);
  EXPECT_EQ(255, green);
  EXPECT_EQ(45, blue);

  ASSERT_TRUE(GetRgbValuesFromString(red, green, blue, " rgb ( 72 , 257 , 47 ) "));
  EXPECT_EQ(72, red);
  EXPECT_EQ(1, green); //rollover 255 --> 255, 256 --> 0, 257 --> 1,...
  EXPECT_EQ(47, blue);

  ASSERT_TRUE(GetRgbValuesFromString(red, green, blue, " rgb ( 72 , 247 , 47 ) "));
  EXPECT_EQ(72, red);
  EXPECT_EQ(247, green);
  EXPECT_EQ(47, blue);

  ASSERT_TRUE(GetRgbValuesFromString(red, green, blue, " rgb ( 000, 0, 000) "));
  EXPECT_EQ(0, red);
  EXPECT_EQ(0, green);
  EXPECT_EQ(0, blue);
}

TEST(GenericToolbox, FastParseTest_StringToDoubleEx01)
{
  using OrthancStone::GenericToolbox::StringToDoubleEx;

  const char* s = "0.0/.123/3/12.5//-43.1";

  int32_t size;
  double r;
  const char* p = s;

  size = StringToDoubleEx(r, p, '/');
  // -->
  // r = 0 and size = 3
  ASSERT_EQ(3, size);
  ASSERT_DOUBLE_EQ(0, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(4, size);
  ASSERT_DOUBLE_EQ(0.123, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(1, size);
  ASSERT_DOUBLE_EQ(3, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(4, size);
  ASSERT_DOUBLE_EQ(12.5, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(0, size);
  ASSERT_DOUBLE_EQ(0, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(5, size);
  ASSERT_DOUBLE_EQ(-43.1, r);

  p += size;
  ASSERT_EQ(0, *p);
}

TEST(GenericToolbox, FastParseTest_StringToDoubleEx02)
{
  using OrthancStone::GenericToolbox::StringToDoubleEx;

  const char* s = "  \t   0.0/.123/3  \t/12.5e-3//-43.1   \t     ";

  int32_t size;
  double r;
  const char* p = s;

  while (*p == ' ' || *p == '\t')
    ++p;

  size = StringToDoubleEx(r, p, '/');
  // -->
  // r = 0 and size = 3
  ASSERT_EQ(3, size);
  ASSERT_DOUBLE_EQ(0, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(4, size);
  ASSERT_DOUBLE_EQ(0.123, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(4, size);
  ASSERT_DOUBLE_EQ(3, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(7, size);
  ASSERT_DOUBLE_EQ(12.5e-3, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(0, size);
  ASSERT_DOUBLE_EQ(0, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(14, size);
  ASSERT_DOUBLE_EQ(-43.1, r);

  p += size;
  ASSERT_EQ(0, *p);
}

TEST(GenericToolbox, FastParseTest_StringToDoubleEx03)
{
  using OrthancStone::GenericToolbox::StringToDoubleEx;

  const char* s = "  \t   0.0/.123/3/12.5e-3//-43.1e-2   \t     ";

  int32_t size;
  double r;
  const char* p = s;

  while (*p == ' ' || *p == '\t')
    ++p;

  size = StringToDoubleEx(r, p, '/');
  // -->
  // r = 0 and size = 3
  ASSERT_EQ(3, size);
  ASSERT_DOUBLE_EQ(0, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(4, size);
  ASSERT_DOUBLE_EQ(0.123, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(1, size);
  ASSERT_DOUBLE_EQ(3, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(7, size);
  ASSERT_DOUBLE_EQ(12.5e-3, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(0, size);
  ASSERT_DOUBLE_EQ(0, r);

  p += (size + 1);
  size = StringToDoubleEx(r, p, '/');
  ASSERT_EQ(17, size);
  ASSERT_DOUBLE_EQ(-43.1e-2, r);

  p += size;
  ASSERT_EQ(0, *p);
}


TEST(GenericToolbox, FastParseTest_GetCharCount)
{
  using OrthancStone::GenericToolbox::GetCharCount;

  ASSERT_EQ(0u, GetCharCount("-1e-22", '\\'));
  ASSERT_EQ(0u, GetCharCount("   -1e-22", '\\'));
  ASSERT_EQ(0u, GetCharCount("   -1e-22   ", '\\'));
  ASSERT_EQ(0u, GetCharCount("-1e-22   ", '\\'));

  ASSERT_EQ(1u, GetCharCount("-1e-2\\2", '\\'));
  ASSERT_EQ(1u, GetCharCount("     -1e-2\\2", '\\'));
  ASSERT_EQ(1u, GetCharCount("-1e-2\\2       ", '\\'));
  ASSERT_EQ(1u, GetCharCount("    -1e-2\\2   ", '\\'));


  ASSERT_EQ(11u, GetCharCount("    -1e-2\\\\3\\12.473\\-2.34e4\\-284\\423.23\\\\0.234423\\.786 \\ 9093\\   ", '\\'));
}


TEST(GenericToolbox, FastParseTest_FastParseVector01)
{
  using OrthancStone::GenericToolbox::FastParseVector;

  OrthancStone::Vector v;

  ASSERT_TRUE(FastParseVector(v, "1.2"));
  ASSERT_EQ(1u, v.size());
  ASSERT_DOUBLE_EQ(1.2, v[0]);

  ASSERT_TRUE(FastParseVector(v, "-1.2e+2"));
  ASSERT_EQ(1u, v.size());
  ASSERT_DOUBLE_EQ(-120.0, v[0]);

  ASSERT_TRUE(FastParseVector(v, "-1e-2\\2"));
  ASSERT_EQ(2u, v.size());
  ASSERT_DOUBLE_EQ(-0.01, v[0]);
  ASSERT_DOUBLE_EQ(2.0, v[1]);

  ASSERT_TRUE(FastParseVector(v, "1.3671875\\1.3671875"));
  ASSERT_EQ(2u, v.size());
  ASSERT_DOUBLE_EQ(1.3671875, v[0]);
  ASSERT_DOUBLE_EQ(1.3671875, v[1]);
}

TEST(GenericToolbox, FastParseTest_FastParseVector02)
{
  using OrthancStone::GenericToolbox::FastParseVector;

  const char* vectorString = "    -1e-2\\\\3\\12.473\\-2.34e4\\-284\\423.23\\\\0.234423\\.786 \\9093\\   ";

  OrthancStone::Vector v;

  ASSERT_TRUE(FastParseVector(v, vectorString));
  ASSERT_EQ(12u, v.size());
  ASSERT_DOUBLE_EQ(-1e-2    , v[ 0]);
  ASSERT_DOUBLE_EQ(0        , v[ 1]);
  ASSERT_DOUBLE_EQ(3        , v[ 2]);
  ASSERT_DOUBLE_EQ(12.473   , v[ 3]);
  ASSERT_DOUBLE_EQ(-2.34e4  , v[ 4]);
  ASSERT_DOUBLE_EQ(-284     , v[ 5]);
  ASSERT_DOUBLE_EQ(423.23   , v[ 6]);
  ASSERT_DOUBLE_EQ(0        , v[ 7]);
  ASSERT_DOUBLE_EQ(0.234423 , v[ 8]);
  ASSERT_DOUBLE_EQ(.786     , v[ 9]);
  ASSERT_DOUBLE_EQ(9093     , v[10]);
  ASSERT_DOUBLE_EQ(0        , v[11]);
}
