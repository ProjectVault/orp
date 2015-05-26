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

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;


public class Conversation extends ActionBarActivity {
    private ArrayAdapter<String> logAdapter = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_conversation);

        final Model model = Model.getModel();

        final Conversation conversationActivity = this;
        model.setConversationActivity(conversationActivity);

        final TextView tvConvo = (TextView)findViewById(R.id.tvConvo);
        final String convo = String.format("%s - %s", model.getUserName(), model.getSeekName());
        tvConvo.setText(model.getUserName());

        final ListView lvLog = (ListView)findViewById(R.id.lvLog);
        logAdapter = new ArrayAdapter<>(this, android.R.layout.simple_list_item_1, android.R.id.text1, model.getChatLog());
        lvLog.setAdapter(logAdapter);
    }

    public void notifyLogChange() {
        if (null != logAdapter) {
            logAdapter.notifyDataSetChanged();
            final ListView lvLog = (ListView)findViewById(R.id.lvLog);
            lvLog.smoothScrollToPosition(lvLog.getMaxScrollAmount());
        }
    }

    @Override
    public void onBackPressed() {
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_conversation, menu);
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

    public void doSay(View view) {
        final EditText etSay = (EditText)findViewById(R.id.etSay);
        final String message = etSay.getText().toString();

        final Model model = Model.getModel();
        Thread sayThread = new Thread(new Runnable() {
            @Override
            public void run() {
                model.say(message);
            }
        });
        sayThread.start();

        etSay.setText("");
    }
}
