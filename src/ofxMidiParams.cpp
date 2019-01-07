//
//  ofxMidiParams.cpp
//  MidiParameter
//
//  Created by Nick Hardeman on 12/28/18.
//

#include "ofxMidiParams.h"

//--------------------------------------------------------------
ofxMidiParams::ofxMidiParams() {
    enableMouseEvents();
    _updatePositions();
}

//--------------------------------------------------------------
ofxMidiParams::~ofxMidiParams() {
    disableMouseEvents();
    disconnect();
    if(bHasUpdateEvent) {
        ofRemoveListener( ofEvents().update, this, &ofxMidiParams::update );
    }
    bHasUpdateEvent = false;
}

//--------------------------------------------------------------
bool ofxMidiParams::connect( int aport, bool abRetryConnection ) {
    
    bTryReconnect = abRetryConnection;
    // print input ports to console
    midiIn.listInPorts();
    
    vector<string> tportNames = midiIn.getInPortList();
    if( aport < 0 || aport >= tportNames.size() ) {
        ofLogNotice( "ofxMidiParams :: connect : port " ) << aport << " is out of range.";
        return false;
    }
    
    // now try to match to a device //
    mDesiredDeviceNameToOpen = tportNames[aport];
    ofLogNotice("ofxMidiParams :: connect : attempting to connect to port: " ) << aport << " for device: " << mDesiredDeviceNameToOpen;
    
    // open port by number (you may need to change this)
    bConnected = midiIn.openPort(aport);
    //midiIn.openPort("IAC Pure Data In");    // by name
    //midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
    
    // don't ignore sysex, timing, & active sense messages,
    // these are ignored by default
    midiIn.ignoreTypes(false, false, false);
    
    if( !bHasUpdateEvent ) {
        ofAddListener( ofEvents().update, this, &ofxMidiParams::update );
    }
    bHasUpdateEvent = true;
    
    // add ofApp as a listener
    if(bConnected) midiIn.addListener(this);
    return bConnected;
}

//--------------------------------------------------------------
bool ofxMidiParams::connect( string aDeviceName, bool abRetryConnection ) {
    mDesiredDeviceNameToOpen = aDeviceName;
    int tport = _getDesiredPortToOpen();
    return connect( tport, abRetryConnection );
}

//--------------------------------------------------------------
void ofxMidiParams::disconnect() {
    if(bConnected) {
        midiIn.closePort();
        midiIn.removeListener(this);
    }
    bConnected = false;
    
}

//--------------------------------------------------------------
bool ofxMidiParams::isConnected() {
    return bConnected && midiIn.isOpen();
}

//--------------------------------------------------------------
bool ofxMidiParams::save( string aXmlFilepath ) {
    if( aXmlFilepath != "" ) {
        mXmlFilePath = aXmlFilepath;
    }
    
    if( mXmlFilePath == "" ) {
        ofFileDialogResult fd = ofSystemSaveDialog("midiparams.xml", "Choose XML file name to save. ");
        if( fd.fileName != "" && fd.bSuccess ) {
            mXmlFilePath = fd.filePath;
        }
    }
    
    ofXml txml;
    auto root = txml.getChild("midiparams");
    if(!root){
        root = txml.appendChild("midiparams");
    }
    
    for( int i = 0; i < mAssocParams.size(); i++ ) {
        auto max = root.appendChild("param");
        max.setAttribute("paramName", mAssocParams[i]->xmlName );
        max.setAttribute("group", mAssocParams[i]->xmlParentName );
        max.setAttribute("midiid", mAssocParams[i]->midiId );
        max.setAttribute("displayName", mAssocParams[i]->displayMidiName );
    }
    
    return txml.save(mXmlFilePath);
}

//--------------------------------------------------------------
bool ofxMidiParams::load( string aXmlFilepath) {
    mXmlFilePath = aXmlFilepath;
    ofXml txml;
    if( txml.load(mXmlFilePath) ) {
        auto paramsXML = txml.find("//midiparams/param");
        for(auto& pxml : paramsXML) {
            string paramName = pxml.getAttribute("paramName").getValue();
            string groupName = pxml.getAttribute("group").getValue();
            int midiid = pxml.getAttribute("midiid").getIntValue();
            string dname = pxml.getAttribute("displayName").getValue();
            // now find the associated param //
            for( auto& ma : mAssocParams ) {
                if( ma->xmlName == paramName && ma->xmlParentName == groupName ) {
                    ma->midiId = midiid;
                    ma->displayMidiName = dname;
                }
            }
        }
        return true;
    }
    ofLogError() << "Couldn't load xml file from " << mXmlFilePath;
    return false;
}

#pragma mark - Update
//--------------------------------------------------------------
void ofxMidiParams::update( ofEventArgs& args ) {
    uint64_t emillis = ofGetElapsedTimeMillis();
    if( emillis >= mNextCheckMillis ) {
        vector<string> tportNames = midiIn.getInPortList();
        bool bopen = midiIn.isOpen();
        if( tportNames.size() == 0 ) {
            if( bopen ) {
                disconnect();
            }
        }
        
        bopen = midiIn.isOpen();
        if(bopen && mDesiredDeviceNameToOpen != "" ) {
            int tport = _getDesiredPortToOpen();
            if( tport < 0 ) disconnect();
        }
        
        
//        cout << "bopen: " << bopen << " port: " << midiIn.getPort() << " | " << ofGetFrameNum() << endl;
        
        if(!bConnected && bTryReconnect ) {
            if(mDesiredDeviceNameToOpen != "") {
                connect( _getDesiredPortToOpen(), true );
            } else {
                connect( 0, true );
            }
        }
        mNextCheckMillis = emillis + 3000;
    }
    
    if( mSelectedParam && mSelectedParam->bNeedsTextPrompt ) {
        mSelectedParam->displayMidiName = ofSystemTextBoxDialog("MIDI Display name.", mSelectedParam->displayMidiName );
        ofStringReplace(mSelectedParam->displayMidiName, "<", "");
        ofStringReplace(mSelectedParam->displayMidiName, ">", "");
        ofStringReplace(mSelectedParam->displayMidiName, "\r", "");
        ofStringReplace(mSelectedParam->displayMidiName, "\n", "");
        ofStringReplace(mSelectedParam->displayMidiName, "\t", "");
        ofStringReplace(mSelectedParam->displayMidiName, "%", "");
        mSelectedParam->bNeedsTextPrompt = false;
        mSelectedParam.reset();
    }
}

#pragma mark - MIDI Messages
//--------------------------------------------------------------
void ofxMidiParams::newMidiMessage(ofxMidiMessage& amsg) {
    
    midiMessages.push_back( amsg );
    
    while(midiMessages.size() > 6) {
        midiMessages.erase(midiMessages.begin());
    }
    
    mMidiMessageHistoryStr = "";
    stringstream ss;
    for( int i = (int)midiMessages.size()-1; i >= 0; i-- ) {
        ofxMidiMessage& message = midiMessages[i];
        string sstatus = ofxMidiMessage::getStatusString(message.status);
        if(message.status == MIDI_CONTROL_CHANGE ) {
            sstatus = "Control";
        }
        ss << sstatus;
        int treq = 10 - sstatus.length();
        for( int k = 0; k < treq; k++ ) {
            ss << " ";
        }
        if(message.status < MIDI_SYSEX) {
            ss << "chan: " << message.channel;
            if(message.status == MIDI_CONTROL_CHANGE) {
                ss << " ctl: " << message.control << " val: " << message.value;// << " pitch: " << message.pitch;
            } else if(message.status == MIDI_PITCH_BEND) {
                ss << "\tval: " << message.value;
            } else {
                ss << "\tpitch: " << message.pitch;// << " value: "<<message.value;// << " control: " << message.control
                ss << "\tvel: " << message.velocity;
            }
        }
        ss << endl;
    }
    if(midiMessages.size() > 0 ) {
        mMidiMessageHistoryStr = ss.str();
    }
    
    
    if(mSelectedParam) {
        mSelectedParam->midiId = getId(amsg);
        string tname = "default";
        if(amsg.status < MIDI_SYSEX) {
//            ss << "chan: " << message.channel;
            if(amsg.status == MIDI_CONTROL_CHANGE) {
                tname = "Control"+ofToString(amsg.control, 0);
            } else if(amsg.status == MIDI_PITCH_BEND) {
                tname = "PitchBend";
            } else {
                tname = "Note "+ofToString(amsg.pitch,0);
            }
        }
        mSelectedParam->displayMidiName = tname;//ofSystemTextBoxDialog("MIDI Display name.", tname );
        mSelectedParam->bNeedsTextPrompt = true;
        mSelectedParam->bListening = false;
//        mSelectedParam.reset();
    }
    
    int tid = getId( amsg );
    // see if we have an assoc //
    for( int i = 0; i < mAssocParams.size(); i++ ) {
        auto& ma = mAssocParams[i];
        if( ma->midiId == tid ) {
            if(amsg.status == MIDI_CONTROL_CHANGE) {
                float pct = ofMap( amsg.value, 0, 127, 0.0, 1.0f, true );
                setParamValue( ma, pct );
            } else if( amsg.status == MIDI_NOTE_OFF) {
                setParamValue( ma, 0.0 );
            } else if( amsg.status == MIDI_NOTE_ON ) {
                setParamValue( ma, 1.0 );
            }
        }
    }
}

//--------------------------------------------------------------
void ofxMidiParams::setParamValue( shared_ptr<MidiParamAssoc>& am, float avaluePct ) {
    if( am->ptype == PTYPE_FLOAT ) {
        ofParameter<float> f = mParamsGroup.getFloat( am->paramIndex );
        f = f.getMin() + ofClamp(avaluePct, 0, 1) * (f.getMax()-f.getMin());
    } else if( am->ptype == PTYPE_BOOL ) {
        ofParameter<bool> f =  mParamsGroup.getBool( am->paramIndex );
        if( avaluePct > 0.0 ) f = true;
        else f = false;
    } else if( am->ptype == PTYPE_INT) {
        ofParameter<int> f =  mParamsGroup.getInt(am->paramIndex);
        f = (float)f.getMin() + ofClamp(avaluePct, 0, 1) * (float)(f.getMax()-f.getMin());
    }
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValue( shared_ptr<MidiParamAssoc>& am ) {
    if( am->ptype == PTYPE_FLOAT ) {
        return mParamsGroup.getFloat( am->paramIndex );
    } else if( am->ptype == PTYPE_BOOL ) {
        ofParameter<bool> f = mParamsGroup.getBool( am->paramIndex );
        return ( f == true ? 1.f : 0.0 );
    } else if( am->ptype == PTYPE_INT) {
        return (float)mParamsGroup.getInt(am->paramIndex);
    }
    return 0.0;
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValue( const shared_ptr<MidiParamAssoc>& am ) const {
    if( am->ptype == PTYPE_FLOAT ) {
        ofParameter<float> f = mParamsGroup.getFloat( am->paramIndex );
        return f;
    } else if( am->ptype == PTYPE_BOOL ) {
        ofParameter<bool> f =  mParamsGroup.getBool( am->paramIndex );
        return ( f == true ? 1.f : 0.0 );
    } else if( am->ptype == PTYPE_INT) {
        ofParameter<int> f =  mParamsGroup.getInt(am->paramIndex);
        return f;
    }
    return 0.0;
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValuePct( shared_ptr<MidiParamAssoc>& am ) {
    if( am->ptype == PTYPE_FLOAT ) {
        ofParameter<float> f = mParamsGroup.getFloat( am->paramIndex );
        return (f-f.getMin()) / (f.getMax()-f.getMin());
    } else if( am->ptype == PTYPE_BOOL ) {
        ofParameter<bool> f =  mParamsGroup.getBool( am->paramIndex );
        return ( f == true ? 1.f : 0.0 );
    } else if( am->ptype == PTYPE_INT) {
        ofParameter<int> f =  mParamsGroup.getInt(am->paramIndex);
        return ((float)f-(float)f.getMin()) / ((float)f.getMax()-(float)f.getMin());
    }
    return 0.0;
}

//--------------------------------------------------------------
float ofxMidiParams::getParamValuePct( const shared_ptr<MidiParamAssoc>& am ) const {
    if( am->ptype == PTYPE_FLOAT ) {
        ofParameter<float> f = mParamsGroup.getFloat( am->paramIndex );
        return (f-f.getMin()) / (f.getMax()-f.getMin());
    } else if( am->ptype == PTYPE_BOOL ) {
        ofParameter<bool> f =  mParamsGroup.getBool( am->paramIndex );
        return ( f == true ? 1.f : 0.0 );
    } else if( am->ptype == PTYPE_INT) {
        ofParameter<int> f =  mParamsGroup.getInt(am->paramIndex);
        return ((float)f-(float)f.getMin()) / ((float)f.getMax()-(float)f.getMin());
    }
    return 0.0;
}

//--------------------------------------------------------------
int ofxMidiParams::getId( ofxMidiMessage& amess ) {
    if( amess.control > 0 ) return amess.control;
    if( amess.pitch > 0 ) return 512 + amess.pitch;
    return amess.pitch + 512.f * (amess.control+1);
}

//--------------------------------------------------------------
void ofxMidiParams::add( ofParameterGroup aparams ) {
    for( int i = 0; i < aparams.size(); i++ ) {
        addParam( aparams.get(i) );
    }
}

//--------------------------------------------------------------
void ofxMidiParams::add( ofParameter<float>& aparam ) {
    addParam(aparam);
}

//--------------------------------------------------------------
void ofxMidiParams::add( ofParameter<bool>& aparam ) {
    addParam(aparam);
}

//--------------------------------------------------------------
void ofxMidiParams::add( ofParameter<int>& aparam ) {
    addParam(aparam);
}

//--------------------------------------------------------------
void ofxMidiParams::addParam( ofAbstractParameter& aparam ) {
    auto mac = make_shared<ofxMidiParams::MidiParamAssoc>();
    mac->paramIndex = mParamsGroup.size();
    
    if(aparam.type() == typeid(ofParameter<int>).name()){
        mac->ptype = PTYPE_INT;
        ofParameter<int> ti = aparam.cast<int>();
        ofParameterGroup pgroup = ti.getFirstParent();
        if( pgroup ) {
            mac->xmlParentName = pgroup.getEscapedName();
        }
    } else if(aparam.type() == typeid(ofParameter<float>).name()){
        mac->ptype = PTYPE_FLOAT;
        ofParameter<float> fi = aparam.cast<float>();
        ofParameterGroup pgroup = fi.getFirstParent();
        if( pgroup ) {
            mac->xmlParentName = pgroup.getEscapedName();
        }
    } else if(aparam.type() == typeid(ofParameter<bool>).name()){
        mac->ptype = PTYPE_BOOL;
        ofParameter<bool> bi = aparam.cast<bool>();
        ofParameterGroup pgroup = bi.getFirstParent();
        if( pgroup ) {
            mac->xmlParentName = pgroup.getEscapedName();
        }
    }
    if( mac->ptype == PTYPE_UNKNOWN ) {
        ofLogNotice("ofxMidiParams :: addParam : unsupported param type");
        return;
    }
    
    mac->xmlName = aparam.getEscapedName();
    
    mParamsGroup.add( aparam );
    mAssocParams.push_back(mac);
    _updatePositions();
}

//--------------------------------------------------------------
void ofxMidiParams::_updatePositions() {
    mHeaderRect.set( 0, 0, width, 24.f );
    mSaveBtnRect.set( mHeaderRect.getRight() - 70, 4, 40, mHeaderRect.height - 8 );
    mMessageRect.set( 0, mHeaderRect.getBottom()+2, width, 16 * 6 );
    for( int i = 0; i < mAssocParams.size(); i++ ) {
        mAssocParams[i]->drawRect.height = 20.f;
        float ty = mMessageRect.getBottom() + 2.f + (float)i*(mAssocParams[i]->drawRect.height+2.f);
        mAssocParams[i]->drawRect.x = 0;
        mAssocParams[i]->drawRect.y = ty;
        mAssocParams[i]->drawRect.width = width;
    }
}

//--------------------------------------------------------------
bool ofxMidiParams::isVisible() {
    return bVisible;
}

//--------------------------------------------------------------
void ofxMidiParams::setVisible( bool ab ) {
    bVisible = ab;
}

//--------------------------------------------------------------
void ofxMidiParams::toggleVisible() {
    bVisible = !bVisible;
}

//--------------------------------------------------------------
void ofxMidiParams::draw() {
    if(!bVisible) return;
    
    ofPushMatrix(); {
        ofTranslate( pos.x, pos.y );
        
        string hstring = mDesiredDeviceNameToOpen;
        if( hstring == "" ) {
            hstring = "No MIDI Device.";
        }
        
        ofSetColor( 30 );
        ofDrawRectangle( mHeaderRect );
        ofSetColor( 225 );
        ofDrawBitmapString( hstring, mHeaderRect.x + 4, mHeaderRect.y + mHeaderRect.height/2 + 6 );
        ofSetColor( 200, 20, 70 );
        if( isConnected() ) {
            ofSetColor( 20, 210, 60 );
        }
        ofDrawCircle( mHeaderRect.width - (mHeaderRect.height / 2), mHeaderRect.height / 2, mHeaderRect.height / 4 );
        
        ofSetColor(80);
        ofDrawRectangle( mSaveBtnRect );
        ofSetColor( 200 );
        ofDrawBitmapString("save", mSaveBtnRect.x + 4, mSaveBtnRect.getCenter().y+4 );
        
        ofSetColor( 10 );
        ofDrawRectangle( mMessageRect );
        string messStr = mMidiMessageHistoryStr;
        if( !isConnected() ) {
            messStr = "No MIDI messages received.";
            
        }
        ofSetColor( 200 );
        ofDrawBitmapString( messStr, mMessageRect.x+8, mMessageRect.y + 18 );
        
        for( int i = 0; i < mAssocParams.size(); i++ ) {
            ofSetColor( 20 );
            auto& ma = mAssocParams[i];
            float val = getParamValuePct( ma );
            ofDrawRectangle( ma->drawRect );
            if( val > 0.0 ) {
                ofSetColor( ofColor::yellowGreen );
                ofDrawRectangle( ma->drawRect.x, ma->drawRect.y, val * ma->drawRect.width, ma->drawRect.height );
            }
            
            if(ma->bListening) {
                ofSetColor( cos(ofGetElapsedTimef() * 3.f) * 50 + 50 );
                ofDrawRectangle( ma->drawRect );
            }
            
            ofSetColor(255);
            float texty = ma->drawRect.y + 14;
            ofDrawBitmapString( mParamsGroup.getName(i), 0.0 + 8, texty );
            
            if( ma->displayMidiName != "" ) {
                float sw = ma->displayMidiName.length() * 8.f;
                ofDrawBitmapString( ma->displayMidiName, ma->drawRect.getRight() - sw - 4, texty );
            } else if( ma->bListening ) {
                string tstr = "Listening";
                float sw = tstr.length() * 8.f;
                ofDrawBitmapString( tstr, ma->drawRect.getRight() - sw - 4, texty );
            }
            
        }
    } ofPopMatrix();
}

//--------------------------------------------------------------
int ofxMidiParams::_getDesiredPortToOpen() {
    if(mDesiredDeviceNameToOpen == "" ) {
        ofLogNotice("ofxMidiParams :: desired device not found." );
        return -1;
    }
    vector<string> tportNames = midiIn.getInPortList();
    for( int i = 0; i < tportNames.size(); i++ ) {
//        cout << i << " port name: " << tportNames[i] << endl;
        // lets first try a hard match and then we can try a loose one //
        if( tportNames[i] == mDesiredDeviceNameToOpen ) {
            return i;
        }
    }
    for( int i = 0; i < tportNames.size(); i++ ) {
        // lets first try a hard match and then we can try a loose one //
        if(  ofIsStringInString(tportNames[i], mDesiredDeviceNameToOpen)) {
            return i;
        }
    }
    
    return -1;
}

#pragma mark - Mouse Events
//--------------------------------------------------------------
void ofxMidiParams::enableMouseEvents() {
    if(!bHasMouseEvents) {
        ofAddListener( ofEvents().mousePressed, this, &ofxMidiParams::onMousePressed );
        ofAddListener( ofEvents().mouseDragged, this, &ofxMidiParams::onMouseDragged );
        ofAddListener( ofEvents().mouseReleased, this, &ofxMidiParams::onMouseReleased );
        ofAddListener( ofEvents().mouseMoved, this, &ofxMidiParams::onMouseMoved );
    }
    bHasMouseEvents = true;
}

//--------------------------------------------------------------
void ofxMidiParams::disableMouseEvents() {
    if(bHasMouseEvents) {
        ofRemoveListener( ofEvents().mousePressed, this, &ofxMidiParams::onMousePressed );
        ofRemoveListener( ofEvents().mouseDragged, this, &ofxMidiParams::onMouseDragged );
        ofRemoveListener( ofEvents().mouseReleased, this, &ofxMidiParams::onMouseReleased );
        ofRemoveListener( ofEvents().mouseMoved, this, &ofxMidiParams::onMouseMoved );
    }
    bHasMouseEvents = false;
}

//--------------------------------------------------------------
void ofxMidiParams::onMousePressed( ofMouseEventArgs& args ) {
    mousePressed( args.x, args.y, args.button );
}

//--------------------------------------------------------------
void ofxMidiParams::onMouseMoved( ofMouseEventArgs& args ) {
    mouseMoved( args.x, args.y );
}

//--------------------------------------------------------------
void ofxMidiParams::onMouseDragged( ofMouseEventArgs& args ) {
    mouseDragged( args.x, args.y, args.button );
}

//--------------------------------------------------------------
void ofxMidiParams::onMouseReleased( ofMouseEventArgs& args ) {
    mouseReleased( args.x, args.y, args.button );
}

//--------------------------------------------------------------
void ofxMidiParams::mouseMoved(int x, int y ) {
    if(!isVisible()) return;
    glm::vec2 mp = glm::vec2(x,y)-pos;
}

//--------------------------------------------------------------
void ofxMidiParams::mouseDragged(int x, int y, int button) {
    if(!isVisible()) return;
    if( bDragging ) {
        pos = glm::vec2(x,y)-mMouseOffset;
    }
}

//--------------------------------------------------------------
void ofxMidiParams::mousePressed(int x, int y, int button) {
    if(!isVisible()) return;
    glm::vec2 mp = glm::vec2(x,y)-pos;
    
    bDragging = false;
    bool bate = false;
    
    if( mSaveBtnRect.inside(mp)) {
        save();
        bate = true;
    }
    
    if( !bate && mHeaderRect.inside(mp) ) {
        mMouseOffset = mp;
        bDragging = true;
    }
    
    bool bHitParam = false;
    if(!bate) {
        for( int i = 0; i < mAssocParams.size(); i++ ) {
            if( mAssocParams[i]->drawRect.inside( mp ) ) {
                mSelectedParam = mAssocParams[i];
                mAssocParams[i]->bListening = true;
                bHitParam = true;
                break;
            }
        }
    }
    if( !bHitParam ) {
        if(mSelectedParam) {
            mSelectedParam->bListening = false;
            mSelectedParam.reset();
        }
    }
}

//--------------------------------------------------------------
void ofxMidiParams::mouseReleased(int x, int y, int button) {
    if(!isVisible()) return;
    bDragging = false;
}





