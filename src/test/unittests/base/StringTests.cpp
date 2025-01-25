/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2014-2016 Symless Ltd.
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "base/String.h"

#include <gtest/gtest.h>

using namespace deskflow;

TEST(StringTests, format_formatWithArguments_formatedString)
{
  const char *format = "%%%{1}=%{2}";
  const char *arg1 = "answer";
  const char *arg2 = "42";

  std::string result = string::format(format, arg1, arg2);

  EXPECT_EQ("%answer=42", result);
}

TEST(StringTests, findReplaceAll_inputString_replacedString)
{
  std::string subject = "foobar";
  std::string find = "bar";
  std::string replace = "baz";

  string::findReplaceAll(subject, find, replace);

  EXPECT_EQ("foobaz", subject);
}

TEST(StringTests, sprintf_formatWithArgument_formatedString)
{
  const char *format = "%s=%d";
  const char *arg1 = "answer";
  int arg2 = 42;

  std::string result = string::sprintf(format, arg1, arg2);

  EXPECT_EQ("answer=42", result);
}

TEST(StringTests, toHex_plaintext_hexString)
{
  EXPECT_EQ("666f6f626172", string::toHex("foobar", 2));
}

TEST(StringTests, fromHexChar_plaintext_hexString)
{
  EXPECT_EQ(-1, string::fromHexChar('z'));
  EXPECT_EQ(0, string::fromHexChar('0'));
  EXPECT_EQ(1, string::fromHexChar('1'));
  EXPECT_EQ(2, string::fromHexChar('2'));
  EXPECT_EQ(3, string::fromHexChar('3'));
  EXPECT_EQ(4, string::fromHexChar('4'));
  EXPECT_EQ(5, string::fromHexChar('5'));
  EXPECT_EQ(6, string::fromHexChar('6'));
  EXPECT_EQ(7, string::fromHexChar('7'));
  EXPECT_EQ(8, string::fromHexChar('8'));
  EXPECT_EQ(9, string::fromHexChar('9'));
  EXPECT_EQ(10, string::fromHexChar('a'));
  EXPECT_EQ(10, string::fromHexChar('A'));
  EXPECT_EQ(11, string::fromHexChar('b'));
  EXPECT_EQ(11, string::fromHexChar('B'));
  EXPECT_EQ(12, string::fromHexChar('c'));
  EXPECT_EQ(12, string::fromHexChar('C'));
  EXPECT_EQ(13, string::fromHexChar('d'));
  EXPECT_EQ(13, string::fromHexChar('D'));
  EXPECT_EQ(14, string::fromHexChar('e'));
  EXPECT_EQ(14, string::fromHexChar('E'));
  EXPECT_EQ(15, string::fromHexChar('f'));
  EXPECT_EQ(15, string::fromHexChar('F'));
}

TEST(StringTests, uppercase_lowercaseInput_uppercaseOutput)
{
  std::string subject = "12foo3BaR";

  string::uppercase(subject);

  EXPECT_EQ("12FOO3BAR", subject);
}

TEST(StringTests, removeChar_inputString_removeAllSpecifiedCharactors)
{
  std::string subject = "foobar";
  const char c = 'o';

  string::removeChar(subject, c);

  EXPECT_EQ("fbar", subject);
}

TEST(StringTests, intToString_inputInt_outputString)
{
  size_t value = 123;

  std::string number = string::sizeTypeToString(value);

  EXPECT_EQ("123", number);
}

TEST(StringTests, stringToUint_inputString_outputInt)
{
  std::string number = "123";

  size_t value = string::stringToSizeType(number);

  EXPECT_EQ(123, value);
}

TEST(StringTests, splitString_twoSeparator_returnThreeParts)
{
  std::string string = "stub1:stub2:stub3";

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(3, results.size());
  EXPECT_EQ("stub1", results[0]);
  EXPECT_EQ("stub2", results[1]);
  EXPECT_EQ("stub3", results[2]);
}

TEST(StringTests, splitString_oneSeparator_returnTwoParts)
{
  std::string string = "stub1:stub2";

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(2, results.size());
  EXPECT_EQ("stub1", results[0]);
  EXPECT_EQ("stub2", results[1]);
}

TEST(StringTests, splitString_noSeparator_returnOriginalString)
{
  std::string string = "stub1";

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(1, results.size());
  EXPECT_EQ("stub1", results[0]);
}

TEST(StringTests, splitString_emptyString_returnEmptyVector)
{
  std::string string;

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(0, results.size());
}

TEST(StringTests, splitString_tailSeparator_returnTwoParts)
{
  std::string string = "stub1:stub2:";

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(2, results.size());
  EXPECT_EQ("stub1", results[0]);
  EXPECT_EQ("stub2", results[1]);
}

TEST(StringTests, splitString_headSeparator_returnTwoParts)
{
  std::string string = ":stub1:stub2";

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(2, results.size());
  EXPECT_EQ("stub1", results[0]);
  EXPECT_EQ("stub2", results[1]);
}

TEST(StringTests, splitString_headAndTailSeparators_returnTwoParts)
{
  std::string string = ":stub1:stub2:";

  std::vector<std::string> results = string::splitString(string, ':');

  EXPECT_EQ(2, results.size());
  EXPECT_EQ("stub1", results[0]);
  EXPECT_EQ("stub2", results[1]);
}
