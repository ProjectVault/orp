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
package orp.dchat;

import android.content.Intent;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.RadioButton;


public class Chat extends ActionBarActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_chat);
    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_chat, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public void doGo(View view) {
        final RadioButton rbEmulated1 = (RadioButton)findViewById(R.id.rbEmulated1);
        final RadioButton rbEmulated2 = (RadioButton)findViewById(R.id.rbEmulated2);
        final RadioButton rbMicroSD = (RadioButton)findViewById(R.id.rbMicroSD);
        final RadioButton rbAlice = (RadioButton)findViewById(R.id.rbAlice);
        final RadioButton rbBob = (RadioButton)findViewById(R.id.rbBob);
        final EditText etHostname = (EditText)findViewById(R.id.etHostname);
        final EditText etRoom = (EditText)findViewById(R.id.etRoom);

        final Model model = Model.getModel();

        if (rbEmulated1.isChecked())
            model.setSelector(Model.PeripheralSelector.EMULATED_1);
        if (rbEmulated2.isChecked())
            model.setSelector(Model.PeripheralSelector.EMULATED_2);
        if (rbMicroSD.isChecked())
            model.setSelector(Model.PeripheralSelector.FAUX_FILE_SYSTEM);

        final String alice_bob_seek = "J&TJHthj7th^hfh";

        if (rbAlice.isChecked()) {
            model.setSeekPhrase(alice_bob_seek);
            model.setSeekName("Bob");
            model.setUserName("Alice");
            model.setUserPass("fewFD$#SAfdszf$Stn4fTH6nHxsD#xzSAdxs");
        }
        if (rbBob.isChecked()) {
            model.setSeekPhrase(alice_bob_seek);
            model.setSeekName("Alice");
            model.setUserName("Bob");
            model.setUserPass("d3D3sFD43sFnU^m&uHtE#AXEd#SdS#rxWC");
        }

        model.setDemoUrl(String.format("http://%s/ur/demo/Demo", etHostname.getText().toString()));
        model.setRoom(Integer.parseInt(etRoom.getText().toString()));

        Intent intent = new Intent(this, Login.class);
        startActivity(intent);
    }
}
