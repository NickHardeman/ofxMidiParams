//
//  ofxMidiParams.h
//  MidiParameter
//
//  Created by Nick Hardeman on 12/28/18.
//

#pragma once
#include "ofMain.h"
#include "ofxMidi.h"

class ofxMidiParams : public ofxMidiListener {
public:
    
    enum ParamType {
        PTYPE_FLOAT=0,
        PTYPE_INT,
        PTYPE_BOOL,
        PTYPE_UNKNOWN
    };
    
    class MidiParamAssoc {
    public:
        int midiId = -1;
        int paramIndex = 0;
        ParamType ptype = PTYPE_UNKNOWN;
        ofRectangle drawRect;
        string displayMidiName = "";
        bool bListening = false;
        bool bNeedsTextPrompt = false;
        string xmlParentName = "";
        string xmlName = "";
    };
    
    ofxMidiParams();
    ~ofxMidiParams();
    
    bool connect( int aport, bool abRetryConnection );
    bool connect( string aDeviceName, bool abRetryConnection );
    void disconnect();
    bool isConnected();
    
    bool save( string aXmlFilepath="" );
    bool load( string aXmlFilepath);
    
    void update( ofEventArgs& args );
    
    void newMidiMessage(ofxMidiMessage& amsg);
    void setParamValue( shared_ptr<MidiParamAssoc>& am, float avaluePct );
    float getParamValue( shared_ptr<MidiParamAssoc>& am );
    float getParamValue( const shared_ptr<MidiParamAssoc>& am ) const;
    
    float getParamValuePct( shared_ptr<MidiParamAssoc>& am );
    float getParamValuePct( const shared_ptr<MidiParamAssoc>& am ) const;
    
    int getId( ofxMidiMessage& amess );
    void add( ofParameterGroup aparams );
    void add( ofParameter<float>& aparam );
    void add( ofParameter<bool>& aparam );
    void add( ofParameter<int>& aparam );
    void addParam( ofAbstractParameter& aparam );
    
    ofxMidiIn& getMidi() { return midiIn; }
    
    bool isVisible();
    void setVisible( bool ab );
    void toggleVisible();
    
    void draw();
    
    void setPosition( float ax, float ay ) { pos = glm::vec2(ax, ay); }
    
    void enableMouseEvents();
    void disableMouseEvents();
    
    
    void onMouseMoved( ofMouseEventArgs& args );
    void onMouseDragged( ofMouseEventArgs& args );
    void onMousePressed( ofMouseEventArgs& args );
    void onMouseReleased( ofMouseEventArgs& args );
    
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    
protected:
    int _getDesiredPortToOpen();
    void _updatePositions();
    
    ofxMidiIn midiIn;
    std::vector<ofxMidiMessage> midiMessages;
    bool bConnected = false;
    bool bHasUpdateEvent = false;
    
    vector< shared_ptr<MidiParamAssoc> > mAssocParams;
    
    ofParameterGroup mParamsGroup;
    string mMidiMessageHistoryStr;
    bool bHasMouseEvents = false;
    bool bVisible = true;
    
    float width = 300.0f;
    glm::vec2 pos, mMouseOffset;
    bool bDragging = false;
    
    ofRectangle mHeaderRect, mMessageRect, mSaveBtnRect;
    
    shared_ptr<MidiParamAssoc> mSelectedParam;
    
    uint64_t mNextCheckMillis = 0;
    string mDesiredDeviceNameToOpen = "";
    bool bTryReconnect = false;
    
    string mXmlFilePath = "";
    
};
