{-|
Module: Tidl.Generate.Java
Description: Generation primitive (Java target) for TIDL

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
module Tidl.Generate.Java where

import Tidl.Cfg
import Tidl.Ast

import Data.List (unfoldr, intercalate)
import Data.Char (toUpper)
import qualified Data.Map as Map
import Text.PrettyPrint
import Tidl.Generate.Pretty

import Tidl.Generate
import Tidl.Generate.Pretty


arrayMaxLength :: String -> String
arrayMaxLength fname = map toUpper fname ++ "_MAX_LENGTH"

arrayFixedLength :: String -> String
arrayFixedLength fname = map toUpper fname ++ "_LENGTH"


data TargetJava = TargetJava

instance GenTarget TargetJava where
 genAll _ ast modname = map (generateTopLevel modname) $ ast_defs ast

generateTopLevel :: String -> TopLevel -> Rendered
generateTopLevel modname tl
 = let pkg = let sep [] = Nothing
                 sep l  = Just . fmap (drop 1) . break (== '.') $ l
             in unfoldr sep modname
       fname = name ++ ".java"
       (name, out) = case tl of
                      TLStruct { struct_name = name, struct_fields = fields }
                       -> (name, generateStruct pkg fname name fields)
                      TLEnum { enum_name = name, enum_labels = labels }
                       -> (name, generateEnum pkg fname name labels)
   in Rendered
        { path = pkg
        , fname = fname
        , contents = out
        }

generateEnum :: [String] -> String -> String -> [EnumLabel] -> Doc
generateEnum pkg fname name labels
 = let hdr = text "/**"
         $+$ text " * @file" <+> text fname
         $+$ text " */"
         $+$ text ""
         $+$ text "package" <+> text (intercalate "." pkg) <> semi
         $+$ text ""
         $+$ text "import orp.orp.TIDL;"
         $+$ text "import java.nio.ByteBuffer;"
         $+$ text "import java.io.ByteArrayOutputStream;"
         $+$ text ""
       lbs = vsepBy (map generateLabel labels) comma <> semi
       fns = text "private" <+> text name <> in_parens (text "int in_value") $+$ in_braces (text "value = in_value;")
             $+$ text ""
             $+$ text "public int getValue()" $+$ in_braces (text "return value;")
             $+$ text ""
             $+$ generateEnumSerial name labels $+$ text "" $+$ generateEnumDeserial name labels
       enm = text "public enum" <+> text name
         $+$ in_braces (lbs $+$ text "private final int value;" $+$ text "" $+$ fns)
       out = hdr $+$ enm $+$ text ""
   in out

generateLabel :: EnumLabel -> Doc
generateLabel (EnumLabel { label_name = name, label_val = val })
 = text name <+> in_parens (integer val)

generateEnumSerial :: String -> [EnumLabel] -> Doc
generateEnumSerial name labels
 = let proto = text "public static void Serialize" <> in_parens (sepBy args comma) <+> text "throws TIDL.TIDLException"
       args = [ text "ByteArrayOutputStream out", text name <+> text "in" ]
       cases = map (generateLabelSerial name) labels
       body = text "switch (in)"
          $+$ in_braces (vcat cases)
   in proto $+$ in_braces body

generateLabelSerial :: String -> EnumLabel -> Doc
generateLabelSerial ename (EnumLabel { label_name = name, label_val = val })
 = text "case" <+> text name <> colon $+$
    (nest 2 $
      text "TIDL.int32_serialize(out," <+> integer val <> text ");"
      $+$ text "break;"
    )

generateEnumDeserial :: String -> [EnumLabel] -> Doc
generateEnumDeserial name labels
 = let proto = text "public static" <+> text name <+> text "Deserialize(ByteBuffer in) throws TIDL.TIDLException"
       cases = map (generateLabelDeserial name) labels
       body = text name <+> text "ret;"
          $+$ text "int v = TIDL.int32_deserialize(in);"
          $+$ text ""
          $+$ text "switch (v)"
          $+$ in_braces (vcat cases $+$ text "default: throw new TIDL.UnrecognizedEnumException(v);")
          $+$ text ""
          $+$ text "return ret;"
   in proto $+$ in_braces body

generateLabelDeserial :: String -> EnumLabel -> Doc
generateLabelDeserial ename (EnumLabel { label_name = name, label_val = val })
 = text "case" <+> integer val <> colon $+$
    (nest 2 $
      text "ret =" <+> text ename <> text "." <> text name <> semi
      $+$ text "break;"
    )

generateStruct :: [String] -> String -> String -> [StructField] -> Doc
generateStruct pkg fname name fields
 = let hdr = text "/**"
         $+$ text " * @file" <+> text fname
         $+$ text " */"
         $+$ text ""
         $+$ text "package" <+> text (intercalate "." pkg) <> semi
         $+$ text ""
         $+$ text "import orp.orp.TIDL;"
         $+$ text "import java.nio.ByteBuffer;"
         $+$ text "import java.io.ByteArrayOutputStream;"
         $+$ text ""
       lns = vsepBy (map generateMemberLength fields) semi <> semi
       mbs = vsepBy (map generateMember fields) semi <> semi
       fns = generateStructSerial name fields $+$ text "" $+$ generateStructDeserial name fields
       enm = text "public class" <+> text name
         $+$ in_braces (lns $+$ text "" $+$ mbs $+$ text "" $+$ fns)
       out = hdr $+$ enm $+$ text ""
   in out

generateMemberLength :: StructField -> Doc
generateMemberLength (StructField{ field_name = fname, field_array = farray })
 = case farray of
    Max l -> text "public static final int" <+> text (arrayMaxLength fname) <+> text "=" <+> integer l
    Fixed l -> text "public static final int" <+> text (arrayFixedLength fname) <+> text "=" <+> integer l
    _ -> empty

generateMember :: StructField -> Doc
generateMember (StructField { field_name = name, field_array = farray, field_type = ty })
 = text "public" <+> generateType ty farray <+> text name

generateType :: FieldType -> ArraySize -> Doc
generateType ftype asize
 = let arr = case asize of
              Scalar -> empty
              _ -> text "[]"
       ty = case ftype of
              FEnum e -> text e
              FStruct s -> text s
              FString -> text "String"
              FInt IKSigned IS8 -> text "byte"
              FInt IKSigned IS16 -> text "short"
              FInt IKSigned IS32 -> text "int"
              FInt IKSigned IS64 -> text "long"
              FInt IKUnsigned IS8 -> text "byte"
              FInt IKUnsigned IS16 -> text "short"
              FInt IKUnsigned IS32 -> text "int"
              FInt IKUnsigned IS64 -> text "long"
   in ty <> arr

generateStructSerial :: String -> [StructField] -> Doc
generateStructSerial name fields
 = let proto = text "public static void Serialize" <> in_parens (sepBy args comma) <+> text "throws TIDL.TIDLException"
       args = [ text "ByteArrayOutputStream out", text name <+> text "in", text "int recdepth" ]
       body = text "if (0 == recdepth) throw new TIDL.RecursionException();"
          $+$ text ""
          $+$ (vcat $ map (\f -> generateFieldSerial (field_name f) (field_type f) (field_array f)) fields)
   in proto $+$ in_braces body

generateFieldSerial :: String -> FieldType -> ArraySize -> Doc
generateFieldSerial name ty Scalar = generateFieldSerial' ("in." ++ name) ty
generateFieldSerial name ty (Max _)
   = text "if (in." <> text name <> text ".length > " <> text (arrayMaxLength name) <> text ") throw new TIDL.ArrayLenException(in." <> text name <> text ".length);"
 $+$ text "TIDL.int32_serialize(out, in." <> text name <> text ".length);"
 $+$ text "for (int i = 0; i < in." <> text name <> text ".length; i++)"
 $+$ (nest 2 $ generateFieldSerial' ("in." ++ name ++ "[i]") ty)
generateFieldSerial name ty (Fixed _)
   = text "if (in." <> text name <> text ".length != " <> text (arrayFixedLength name) <> text ") throw new TIDL.ArrayLenException(in." <> text name <> text ".length);"
 $+$ text "for (int i = 0; i < " <> text (arrayFixedLength name) <> text "; i++)"
 $+$ (nest 2 $ generateFieldSerial' ("in." ++ name ++ "[i]") ty)

generateFieldSerial' :: String -> FieldType -> Doc
generateFieldSerial' name (FEnum e) = text e <> text ".Serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FStruct s) = text s <> text ".Serialize(out," <+> text name <> text ", recdepth-1);"
generateFieldSerial' name FString = text "TIDL.string_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKSigned IS8) = text "TIDL.int8_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKSigned IS16) = text "TIDL.int16_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKSigned IS32) = text "TIDL.int32_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKSigned IS64) = text "TIDL.int64_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKUnsigned IS8) = text "TIDL.uint8_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKUnsigned IS16) = text "TIDL.uint16_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKUnsigned IS32) = text "TIDL.uint32_serialize(out," <+> text name <> text ");"
generateFieldSerial' name (FInt IKUnsigned IS64) = text "TIDL.uint64_serialize(out," <+> text name <> text ");"

generateStructDeserial :: String -> [StructField] -> Doc
generateStructDeserial name fields
 = let proto = text "public static" <+> text name <+> text "Deserialize(ByteBuffer in, int recdepth) throws TIDL.TIDLException"
       body = text "if (0 == recdepth) throw new TIDL.RecursionException();"
          $+$ text ""
          $+$ text name <+> text "ret = new" <+> text name <> text "();"
          $+$ text ""
          $+$ (vcat $ map (\f -> generateFieldDeserial (field_name f) (field_type f) (field_array f)) fields)
          $+$ text ""
          $+$ text "return ret;"
   in proto $+$ in_braces body

generateFieldDeserial :: String -> FieldType -> ArraySize -> Doc
generateFieldDeserial name ty Scalar = generateFieldDeserial' ("ret." ++ name) ty
generateFieldDeserial name ty (Fixed _)
 =     text "ret." <> text name <+> text "= new" <+> generateType ty Scalar <> text "[" <> text (arrayFixedLength name) <> text "];"
   $+$ text "for (int i = 0; i < " <> text (arrayFixedLength name) <> text "; i++)"
   $+$ (nest 2 $ generateFieldDeserial' ("ret." ++ name ++ "[i]") ty)
generateFieldDeserial name ty (Max _)
 =     text "int l = TIDL.int32_deserialize(in);"
   $+$ text ("if (l > " ++ arrayMaxLength name ++ ") throw new TIDL.ArrayLenException(l);")
   $+$ text "ret." <> text name <+> text "= new" <+> generateType ty Scalar <> text "[l];"
   $+$ text "for (int i = 0; i < l; i++)"
   $+$ (nest 2 $ generateFieldDeserial' ("ret." ++ name ++ "[i]") ty)

generateFieldDeserial' :: String -> FieldType -> Doc
generateFieldDeserial' name (FEnum e) = text name <+> text "=" <+> text e <> text ".Deserialize(in);"
generateFieldDeserial' name (FStruct s) = text name <+> text "=" <+> text s <> text ".Deserialize(in, recdepth-1);"
generateFieldDeserial' name FString = text name <+> text "=" <+> text "TIDL.string_deserialize(in);"
generateFieldDeserial' name (FInt IKSigned IS8) = text name <+> text "=" <+> text "TIDL.int8_deserialize(in);"
generateFieldDeserial' name (FInt IKSigned IS16) = text name <+> text "=" <+> text "TIDL.int16_deserialize(in);"
generateFieldDeserial' name (FInt IKSigned IS32) = text name <+> text "=" <+> text "TIDL.int32_deserialize(in);"
generateFieldDeserial' name (FInt IKSigned IS64) = text name <+> text "=" <+> text "TIDL.int64_deserialize(in);"
generateFieldDeserial' name (FInt IKUnsigned IS8) = text name <+> text "=" <+> text "TIDL.uint8_deserialize(in);"
generateFieldDeserial' name (FInt IKUnsigned IS16) = text name <+> text "=" <+> text "TIDL.uint16_deserialize(in);"
generateFieldDeserial' name (FInt IKUnsigned IS32) = text name <+> text "=" <+> text "TIDL.uint32_deserialize(in);"
generateFieldDeserial' name (FInt IKUnsigned IS64) = text name <+> text "=" <+> text "TIDL.uint64_deserialize(in);"
