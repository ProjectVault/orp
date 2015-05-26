{-|
Module: Tidl.Ast
Description: Abstract syntax for TIDL

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
module Tidl.Ast where

import Text.PrettyPrint

data AST
 = AST
    { ast_defs :: [TopLevel]
    }
 deriving Show

data TopLevel
 = TLEnum
    { enum_name :: String
    , enum_labels :: [EnumLabel]
    }
 | TLStruct
    { struct_name :: String
    , struct_fields :: [StructField]
    }
 deriving Show

data EnumLabel
 = EnumLabel
    { label_name :: String
    , label_val :: Integer
    }
 deriving Show

data ArraySize = Scalar | Fixed Integer | Max Integer
 deriving Show

data StructField
 = StructField
    { field_name :: String
    , field_array :: ArraySize
    , field_type :: FieldType
    }
 deriving Show

isResizable :: StructField -> Bool
isResizable (StructField{ field_array = Max _ }) = True
isResizable _ = False

data IntKind = IKSigned | IKUnsigned
 deriving Show

data IntSize = IS8 | IS16 | IS32 | IS64
 deriving Show

data FieldType
 = FEnum String
 | FStruct String
 | FString
 | FInt IntKind IntSize
 deriving Show

