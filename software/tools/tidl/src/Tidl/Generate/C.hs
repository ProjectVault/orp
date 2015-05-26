{-|
Module: Tidl.Generate.C
Description: Generation primitive (C target) for TIDL

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
module Tidl.Generate.C where

import Tidl.Cfg
import Tidl.Ast
import Control.Monad
import Data.Char

import qualified Data.Map.Strict as Map
import Text.PrettyPrint

import Tidl.Generate
import Tidl.Generate.Pretty

data TargetC = TargetC

instance GenTarget TargetC where
 genAll _ ast modname = [header ast modname, impl ast modname]

arrayLenFixed :: String -> String -> String
arrayLenFixed sname fname = map toUpper sname ++ "_" ++ map toUpper fname ++ "_LENGTH"

arrayLenMax :: String -> String -> String
arrayLenMax sname fname = map toUpper sname ++ "_" ++ map toUpper fname ++ "_MAX_LENGTH"

arrayLen :: String -> String
arrayLen fname = fname ++ "_length"

enumSerialize ename = ename ++ "_serialize"
enumDeserialize ename = ename ++ "_deserialize"
structInit sname = sname ++ "_init"
structCleanup sname = sname ++ "_cleanup"
structSerialize sname = sname ++ "_serialize"
structDeserialize sname = sname ++ "_deserialize"
arrayResize sname fname = sname ++ "_" ++ fname ++ "_resize"

intType :: IntKind -> IntSize -> String
intType IKSigned IS8 = "int8_t"
intType IKSigned IS16 = "int16_t"
intType IKSigned IS32 = "int32_t"
intType IKSigned IS64 = "int64_t"
intType IKUnsigned IS8 = "uint8_t"
intType IKUnsigned IS16 = "uint16_t"
intType IKUnsigned IS32 = "uint32_t"
intType IKUnsigned IS64 = "uint64_t"

fieldType ftype
 = case ftype of
    FEnum ename -> "enum " ++ ename
    FStruct sname -> "struct " ++ sname
    FString -> "char*"
    FInt ik is -> intType ik is

header :: AST -> String -> Rendered
header ast modname
 = let hdr_def    = "__" ++ (map toUpper modname) ++ "_H__"
       out_h_name = modname ++ ".h"
   in Rendered
       { path = ["include"]
       , fname = out_h_name
       , contents = runPrinter $
           do sayln' $ "/**"
              sayln' $ " * @file " ++ out_h_name
              sayln' $ " */"
              sayln' $ ""
              sayln' $ "#ifndef " ++ hdr_def
              sayln' $ "#define " ++ hdr_def
              sayln' $ ""
              sayln' $ ""
              sayln' $ "#include <tidl.h>"
              sayln' $ ""
              sayln' $ ""
              sayln' $ "/* Start forward declarations */"
              sayln' $ ""
              mapM_ forwardDeclaration (ast_defs ast)
              sayln' $ ""
              sayln' $ "/* End forward declarations */"
              sayln' $ ""
              sayln' $ ""
              sayln' $ "/* Start array length constants */"
              sayln' $ ""
              mapM_ arrayLengthConstant (ast_defs ast)
              sayln' $ ""
              sayln' $ "/* End array length constants */"
              sayln' $ ""
              sayln' $ ""
              sayln' $ "/* Start type definitions */"
              sayln' $ ""
              mapM_ (\x -> typeDefinition x >> sayln' "") (ast_defs ast)
              sayln' $ ""
              sayln' $ "/* End type definitions */"
              sayln' $ ""
              sayln' $ ""
              sayln' $ "/* Start function prototypes */"
              sayln' $ ""
              mapM_ (\x -> functionProtos x >> sayln' "") (ast_defs ast)
              sayln' $ ""
              sayln' $ "/* End function prototypes */"
              sayln' $ ""
              sayln' $ ""
              sayln' $ "#endif /* " ++ hdr_def ++ " */"
              sayln' $ ""
       }

forwardDeclaration :: TopLevel -> Printer ()
forwardDeclaration (TLEnum{ enum_name = ename }) = sayln' $ "enum " ++ ename ++ ";"
forwardDeclaration (TLStruct{ struct_name = sname }) = sayln' $ "struct " ++ sname ++ ";"

arrayLengthConstant :: TopLevel -> Printer ()
arrayLengthConstant (TLEnum{ }) = return ()
arrayLengthConstant (TLStruct{ struct_name = sname, struct_fields = fields })
 = mapM_ arrayLengthConstant' fields
 where arrayLengthConstant' field
        = case field of
            StructField{ field_array = Scalar }
              -> return ()
            StructField{ field_name = fname, field_array = Fixed l }
              -> sayln' $ "#define " ++ arrayLenFixed sname fname ++ " " ++ show l
            StructField{ field_name = fname, field_array = Max l }
              -> sayln' $ "#define " ++ arrayLenMax sname fname ++ " " ++ show l

typeDefinition :: TopLevel -> Printer ()
typeDefinition (TLEnum{ enum_name = ename, enum_labels = labels })
 = do sayln' $ "enum " ++ ename ++ " {"
      mapM_ (\EnumLabel{ label_name = lname, label_val = lval }
              -> sayln' $ "  " ++ lname ++ " = " ++ show lval ++ ","
            ) labels
      sayln' $ "};"
typeDefinition (TLStruct{ struct_name = sname, struct_fields = fields })
 = do sayln' $ "struct " ++ sname ++ " {"
      mapM_ fieldDefinition fields
      sayln' $ "};"
 where fieldDefinition (StructField{ field_name = fname, field_array = farray, field_type = ftype })
         = case farray of
            Scalar   -> sayln' $ "  " ++ fieldType ftype ++ " " ++ fname ++ ";"
            Fixed _  -> sayln' $ "  " ++ fieldType ftype ++ " " ++ fname ++ "[" ++ arrayLenFixed sname fname ++ "];"
            Max _ -> do sayln' $ "  " ++ intType IKUnsigned IS32 ++ " " ++ arrayLen fname ++ ";"
                        sayln' $ "  " ++ fieldType ftype ++ "* " ++ fname ++ ";"


functionProtos :: TopLevel -> Printer ()
functionProtos (TLEnum{ enum_name = ename })
 = do sayln' $ enumSerializeProto ename ++ ";"
      sayln' $ enumDeserializeProto ename ++ ";"
functionProtos (TLStruct{ struct_name = sname, struct_fields = fields })
 = do sayln' $ structInitProto sname ++ ";"
      sayln' $ structCleanupProto sname ++ ";"
      sayln' $ structSerializeProto sname ++ ";"
      sayln' $ structDeserializeProto sname ++ ";"
      mapM_ (\f -> sayln' $ arrayResizeProto sname f ++ ";") (filter isResizable fields)

enumSerializeProto ename = "int " ++ enumSerialize ename ++ "(uint8_t *out, uint32_t out_len, uint32_t *pos, enum " ++ ename ++ " in)"
enumDeserializeProto ename = "int " ++ enumDeserialize ename ++ "(uint8_t *in, uint32_t in_len, uint32_t *pos, enum " ++ ename ++ " *out)"
structInitProto sname = "void " ++ structInit sname ++ "(struct " ++ sname ++ " *in)"
structCleanupProto sname = "void " ++ structCleanup sname ++ "(struct " ++ sname ++ " *in)"
structSerializeProto sname = "int " ++ structSerialize sname ++ "(uint8_t *out, uint32_t out_len, uint32_t *pos, struct " ++ sname ++ " *in)"
structDeserializeProto sname = "int " ++ structDeserialize sname ++ "(uint8_t *in, uint32_t in_len, uint32_t *pos, struct " ++ sname ++ " *out)"
arrayResizeProto sname (StructField{ field_name = fname, field_array = farray })
 = case farray of
    Max _ -> "int " ++ arrayResize sname fname ++ "(struct " ++ sname ++ " *in, " ++ intType IKUnsigned IS32 ++ " nsize)"
    _ -> undefined


impl :: AST -> String -> Rendered
impl ast modname
 = let out_h_name = modname ++ ".h"
       out_c_name = modname ++ ".c"
   in Rendered
       { path = []
       , fname = out_c_name
       , contents = runPrinter $
           do sayln' $ "/**"
              sayln' $ " * @file " ++ out_c_name
              sayln' $ " */"
              sayln' $ ""
              sayln' $ ""
              sayln' $ "#include <string.h>"
              sayln' $ "#include <stdlib.h>"
              sayln' $ "#include <" ++ out_h_name ++ ">"
              sayln' $ ""
              sayln' $ ""
              mapM_ (\x -> functionImpls x >> sayln' "") (ast_defs ast)
              sayln' $ ""
              sayln' $ ""
       }

functionImpls :: TopLevel -> Printer ()
functionImpls (TLEnum{ enum_name = ename, enum_labels = labels })
 = do enumSerializeImpl ename labels >> sayln' ""
      enumDeserializeImpl ename labels >> sayln' ""
functionImpls (TLStruct{ struct_name = sname, struct_fields = fields })
 = do structInitImpl sname fields >> sayln' ""
      structCleanupImpl sname fields >> sayln' ""
      structSerializeImpl sname fields >> sayln' ""
      structDeserializeImpl sname fields >> sayln' ""
      mapM_ (\f -> arrayResizeImpl sname f >> sayln' "") (filter isResizable fields)

serialize :: FieldType -> String
serialize (FEnum ename) = enumSerialize ename
serialize (FStruct sname) = structSerialize sname
serialize FString = "string_serialize"
serialize (FInt IKSigned IS8) = "int8_serialize"
serialize (FInt IKSigned IS16) = "int16_serialize"
serialize (FInt IKSigned IS32) = "int32_serialize"
serialize (FInt IKSigned IS64) = "int64_serialize"
serialize (FInt IKUnsigned IS8) = "uint8_serialize"
serialize (FInt IKUnsigned IS16) = "uint16_serialize"
serialize (FInt IKUnsigned IS32) = "uint32_serialize"
serialize (FInt IKUnsigned IS64) = "uint64_serialize"

deserialize :: FieldType -> String
deserialize (FEnum ename) = enumDeserialize ename
deserialize (FStruct sname) = structDeserialize sname
deserialize FString = "string_deserialize"
deserialize (FInt IKSigned IS8) = "int8_deserialize"
deserialize (FInt IKSigned IS16) = "int16_deserialize"
deserialize (FInt IKSigned IS32) = "int32_deserialize"
deserialize (FInt IKSigned IS64) = "int64_deserialize"
deserialize (FInt IKUnsigned IS8) = "uint8_deserialize"
deserialize (FInt IKUnsigned IS16) = "uint16_deserialize"
deserialize (FInt IKUnsigned IS32) = "uint32_deserialize"
deserialize (FInt IKUnsigned IS64) = "uint64_deserialize"

enumSerializeImpl ename labels
 = do sayln' $ enumSerializeProto ename ++ " {"
      sayln' $ "  int ret = 0;"
      sayln' $ "  "
      sayln' $ "  switch (in) {"
      flip mapM_ labels $ \EnumLabel{ label_name = lname, label_val = lval }
        -> do sayln' $ "    case " ++ lname ++ ":"
              sayln' $ "      ret = " ++ serialize (FInt IKSigned IS32) ++ "(out, out_len, pos, " ++ show lval ++ ");"
              sayln' $ "      break;"
      sayln' $ "    default: ret = -1;"
      sayln' $ "  }"
      sayln' $ "  "
      sayln' $ "  return ret;"
      sayln' $ "}"

enumDeserializeImpl ename labels
 = do sayln' $ enumDeserializeProto ename ++ " {"
      sayln' $ "  int ret = 0;"
      sayln' $ "  "
      sayln' $ "  " ++ intType IKSigned IS32 ++ " temp = 0;"
      sayln' $ "  ret = " ++ deserialize (FInt IKSigned IS32) ++ "(in, in_len, pos, &temp);"
      sayln' $ "  if (ret) return ret;"
      sayln' $ "  switch (temp) {"
      flip mapM_ labels $ \EnumLabel{ label_name = lname, label_val = lval }
        -> do sayln' $ "    case " ++ show lval ++ ":"
              sayln' $ "      *out = " ++ lname ++ ";"
              sayln' $ "      break;"
      sayln' $ "    default: ret = -1;"
      sayln' $ "  }"
      sayln' $ "  "
      sayln' $ "  return ret;"
      sayln' $ "}"

structInitImpl sname fields
 = do sayln' $ structInitProto sname ++ " {"
      sayln' $ "  memset(in, 0, sizeof(*in));"
      sayln' $ "}"

structCleanupImpl sname fields
 = do sayln' $ structCleanupProto sname ++ " {"
      flip mapM_ fields $ \(StructField{ field_name = fname, field_array = farray, field_type = ftype })
        -> case farray of
            Scalar ->
              case ftype of
                FStruct csname ->
                  sayln' $ "  " ++ structCleanup csname ++ "(&in->" ++ fname ++ ");";
                FString ->
                  do sayln' $ "  if (in->" ++ fname ++ ") {"
                     sayln' $ "    free(in->" ++ fname ++ ");"
                     sayln' $ "    in->" ++ fname ++ " = NULL;"
                     sayln' $ "  }"
                _ -> return ()
            Fixed _ ->
              case ftype of
                FStruct csname ->
                  do sayln' $ "  for (int i = 0; i < " ++ arrayLenFixed sname fname ++ "; i++)"
                     sayln' $ "    " ++ structCleanup csname ++ "(&in->" ++ fname ++ "[i]);";
                FString ->
                  do sayln' $ "  for (int i = 0; i < " ++ arrayLenFixed sname fname ++ "; i++) {"
                     sayln' $ "    if (in->" ++ fname ++ "[i]) {"
                     sayln' $ "      free(in->" ++ fname ++ "[i]);"
                     sayln' $ "      in->" ++ fname ++ "[i] = NULL;"
                     sayln' $ "    }"
                     sayln' $ "  }"
                _ -> return ()
            Max _ ->
              do case ftype of
                  FStruct csname ->
                    do sayln' $ "  for (int i = 0; i < in->" ++ arrayLen fname ++ "; i++)"
                       sayln' $ "    " ++ structCleanup csname ++ "(&in->" ++ fname ++ "[i]);";
                  FString ->
                    do sayln' $ "  for (int i = 0; i < in->" ++ arrayLen fname ++ "; i++) {"
                       sayln' $ "    if (in->" ++ fname ++ "[i]) {"
                       sayln' $ "      free(in->" ++ fname ++ "[i]);"
                       sayln' $ "      in->" ++ fname ++ "[i] = NULL;"
                       sayln' $ "    }"
                       sayln' $ "  }"
                  _ -> return ()
                 sayln' $ "  if (in->" ++ fname ++ ") {"
                 sayln' $ "    free(in->" ++ fname ++ ");"
                 sayln' $ "    in->" ++ fname ++ " = NULL;"
                 sayln' $ "  }"
      sayln' $ "  " ++ structInit sname ++ "(in);"
      sayln' $ "}"

structSerializeImpl sname fields
 = do sayln' $ structSerializeProto sname ++ " {"
      sayln' $ "  int ret = 0;"
      sayln' $ "  "
      flip mapM_ fields $ \(StructField{ field_name = fname, field_array = farray, field_type = ftype })
        -> case farray of
            Scalar ->
              case ftype of
                FStruct csname ->
                  do sayln' $ "  ret = " ++ serialize ftype ++ "(out, out_len, pos, &in->" ++ fname ++ ");"
                     sayln' $ "  if (ret) return ret;"
                _ ->
                  do sayln' $ "  ret = " ++ serialize ftype ++ "(out, out_len, pos, in->" ++ fname ++ ");"
                     sayln' $ "  if (ret) return ret;"
            Fixed _ ->
              do sayln' $ "  for (int i = 0; i < " ++ arrayLenFixed sname fname ++ "; i++) {"
                 case ftype of
                    FStruct csname ->
                      do sayln' $ "    ret = " ++ serialize ftype ++ "(out, out_len, pos, &in->" ++ fname ++ "[i]);"
                         sayln' $ "    if (ret) return ret;"
                    _ ->
                      do sayln' $ "    ret = " ++ serialize ftype ++ "(out, out_len, pos, in->" ++ fname ++ "[i]);"
                         sayln' $ "    if (ret) return ret;"
                 sayln' $ "  }"
            Max _ ->
              do sayln' $ "  ret = " ++ serialize (FInt IKUnsigned IS32) ++ "(out, out_len, pos, in->" ++ arrayLen fname ++ ");"
                 sayln' $ "  if (ret) return ret;"
                 sayln' $ "  for (int i = 0; i < in->" ++ arrayLen fname ++ "; i++) {"
                 case ftype of
                    FStruct csname ->
                      do sayln' $ "    ret = " ++ serialize ftype ++ "(out, out_len, pos, &in->" ++ fname ++ "[i]);"
                         sayln' $ "    if (ret) return ret;"
                    _ ->
                      do sayln' $ "    ret = " ++ serialize ftype ++ "(out, out_len, pos, in->" ++ fname ++ "[i]);"
                         sayln' $ "    if (ret) return ret;"
                 sayln' $ "  }"
      sayln' $ "  "
      sayln' $ "  return ret;"
      sayln' $ "}"

structDeserializeImpl sname fields
 = do sayln' $ structDeserializeProto sname ++ " {"
      sayln' $ "  int ret = 0;"
      when (any isResizable fields) (sayln' $ "  " ++ intType IKUnsigned IS32 ++ " temp = 0;")
      sayln' $ "  "
      flip mapM_ fields $ \(StructField{ field_name = fname, field_array = farray, field_type = ftype })
        -> case farray of
            Scalar ->
              do sayln' $ "  ret = " ++ deserialize ftype ++ "(in, in_len, pos, &out->" ++ fname ++ ");"
                 sayln' $ "  if (ret) return ret;"
            Fixed _ ->
              do sayln' $ "  for (int i = 0; i < " ++ arrayLenFixed sname fname ++ "; i++) {"
                 sayln' $ "    ret = " ++ deserialize ftype ++ "(in, in_len, pos, &out->" ++ fname ++ "[i]);"
                 sayln' $ "    if (ret) return ret;"
                 sayln' $ "  }"
            Max _ ->
              do sayln' $ "  ret = " ++ deserialize (FInt IKUnsigned IS32) ++ "(in, in_len, pos, &temp);"
                 sayln' $ "  if (ret) return ret;"
                 sayln' $ "  ret = " ++ arrayResize sname fname ++ "(out, temp);"
                 sayln' $ "  if (ret) return ret;"
                 sayln' $ "  for (int i = 0; i < out->" ++ arrayLen fname ++ "; i++) {"
                 sayln' $ "    ret = " ++ deserialize ftype ++ "(in, in_len, pos, &out->" ++ fname ++ "[i]);"
                 sayln' $ "    if (ret) return ret;"
                 sayln' $ "  }"
      sayln' $ "  "
      sayln' $ "  return ret;"
      sayln' $ "}"

arrayResizeImpl sname field@(StructField{ field_name = fname, field_array = farray, field_type = ftype })
 = case farray of
    Scalar -> undefined
    Fixed _ -> undefined
    Max _ -> do sayln' $ arrayResizeProto sname field ++ " {"
                sayln' $ "  if (nsize > " ++ arrayLenMax sname fname ++ ") return -1;"
                sayln' $ "  "
                sayln' $ "  if (0 == nsize) {"
                sayln' $ "    if (in->" ++ fname ++ ") {"
                sayln' $ "      free(in->" ++ fname ++ ");"
                sayln' $ "      in->" ++ fname ++ " = NULL;"
                sayln' $ "      in->" ++ arrayLen fname ++ " = 0;"
                sayln' $ "    }"
                sayln' $ "    return 0;"
                sayln' $ "  }"
                sayln' $ "  "
                sayln' $ "  " ++ fieldType ftype ++ "* temp = (" ++ fieldType ftype ++ "*)malloc(sizeof(*temp) * nsize);"
                sayln' $ "  if (!temp) return -1;"
                sayln' $ "  memset(temp, 0, sizeof(*temp) * nsize);"
                sayln' $ "  "
                sayln' $ "  " ++ intType IKUnsigned IS32 ++ " csize = in->" ++ arrayLen fname ++ ";"
                sayln' $ "  if (csize > nsize) csize = nsize;"
                sayln' $ "  memcpy(temp, in->" ++ fname ++ ", csize);"
                sayln' $ "  "
                sayln' $ "  free(in->" ++ fname ++ ");"
                sayln' $ "  in->" ++ fname ++ " = temp;";
                sayln' $ "  in->" ++ arrayLen fname ++ " = nsize;";
                sayln' $ "  return 0;"
                sayln' $ "}"
