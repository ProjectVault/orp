{-|
Module: Tidl.Parser
Description: Parser for TIDL input into TIDL AST.

   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
-}
module Tidl.Parser where

import Tidl.Ast

import Text.ParserCombinators.Parsec
import qualified Text.Parsec.Token as P
import Text.Parsec.Language (javaStyle)

lexer = P.makeTokenParser javaStyle
  { P.reservedNames = [ "string", "enum", "struct"
                      , "array", "max"
                      , "int8", "int16", "int32", "int64"
                      , "uint8", "uint16", "uint32", "uint64"
                      ]
  , P.caseSensitive = True
  }

whiteSpace = P.whiteSpace lexer
semi = P.semi lexer
comma = P.comma lexer
symbol = P.symbol lexer
identifier = P.identifier lexer
reserved = P.reserved lexer
braces = P.braces lexer
integer = P.integer lexer

tidlFile
 = do whiteSpace
      tls <- many parseTopLevel
      return $ AST tls

parseTopLevel
   = parseStruct
 <|> parseEnum

parseStruct
 = do reserved "struct"
      name <- identifier
      fields <- braces $ many parseField
      return $ TLStruct name fields

parseField
 = do farray <- parseFieldArray
      ftype <- parseFieldType
      fname <- identifier
      semi
      return $ StructField{ field_name = fname
                          , field_array = farray
                          , field_type = ftype
                          }

parseFieldArray
 = parseFieldArray' <|> return Scalar
 where parseFieldArray'
        = do reserved "array"
             symbol "["
             asize <- (reserved "max" >> integer >>= return . Max)
                      <|> (integer >>= return . Fixed)
             symbol "]"
             return asize

parseFieldType
   = choice
      [ reserved "enum" >> identifier >>= return . FEnum
      , reserved "struct" >> identifier >>= return . FStruct
      , reserved "string" >> return FString
      , reserved "int8" >> return (FInt IKSigned IS8)
      , reserved "int16" >> return (FInt IKSigned IS16)
      , reserved "int32" >> return (FInt IKSigned IS32)
      , reserved "int64" >> return (FInt IKSigned IS64)
      , reserved "uint8" >> return (FInt IKUnsigned IS8)
      , reserved "uint16" >> return (FInt IKUnsigned IS16)
      , reserved "uint32" >> return (FInt IKUnsigned IS32)
      , reserved "uint64" >> return (FInt IKUnsigned IS64)
      ]

parseEnum
 = do reserved "enum"
      name <- identifier
      labels <- braces $ many parseLabel
      return $ TLEnum name labels

parseLabel
 = do name <- identifier
      symbol "="
      val <- integer
      comma
      return $ EnumLabel name val

parseTidl :: String -> Either ParseError AST
parseTidl = parse tidlFile "(unknown)"
