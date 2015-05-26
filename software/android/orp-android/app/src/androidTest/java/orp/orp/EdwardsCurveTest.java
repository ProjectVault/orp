/*
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
*/
package orp.orp;

import android.test.InstrumentationTestCase;

import orp.orp.crypto.EdwardsCurve;
import orp.orp.crypto.EdwardsPoint;
import orp.orp.crypto.PrimeField;

public class EdwardsCurveTest extends InstrumentationTestCase {
    public void testAddIdentity() {
        EdwardsCurve curve = EdwardsCurve.curveE521();
        PrimeField Fp = curve.Fp;
        EdwardsPoint identity = curve.id;
        EdwardsPoint pt = curve.g;
        EdwardsPoint sum = pt.Add(identity);
        assertEquals(Fp.mul(pt.x, Fp.inv(pt.z)), Fp.mul(sum.x, Fp.inv(sum.z)));
        assertEquals(Fp.mul(pt.y, Fp.inv(pt.z)), Fp.mul(sum.y, Fp.inv(sum.z)));
    }

    public void testDoubleIdentity() {
        EdwardsCurve curve = EdwardsCurve.curveE521();
        PrimeField Fp = curve.Fp;
        EdwardsPoint pt1 = curve.id;
        EdwardsPoint pt2 = pt1.Double();
        assertEquals(Fp.mul(pt1.x, Fp.inv(pt1.z)), Fp.mul(pt2.x, Fp.inv(pt2.z)));
        assertEquals(Fp.mul(pt1.y, Fp.inv(pt1.z)), Fp.mul(pt2.y, Fp.inv(pt2.z)));
    }

    public void testLadder() {
        EdwardsCurve curve = EdwardsCurve.curveE521();
        PrimeField Fp = curve.Fp;
        EdwardsPoint id = curve.id;
        EdwardsPoint pt = curve.g;
        EdwardsPoint maybeIdentity = pt.Montgomery(curve.order);
        assertEquals(Fp.mul(id.x, Fp.inv(id.z)), Fp.mul(maybeIdentity.x, Fp.inv(maybeIdentity.z)));
        assertEquals(Fp.mul(id.y, Fp.inv(id.z)), Fp.mul(maybeIdentity.y, Fp.inv(maybeIdentity.z)));
    }
}
