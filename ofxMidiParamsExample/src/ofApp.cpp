#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    padParams.setName( "Pad Params" );
    padParams.add( pad1.set("Pad1", false ));
    padParams.add( pad2.set("Pad2", false ));
    padParams.add( pad3.set("Pad3", false ));
    padParams.add( pad4.set("Pad4", false ));
    padParams.add( pad5.set("Pad5", false ));
    padParams.add( pad6.set("Pad6", false ));
    padParams.add( pad7.set("Pad7", false ));
    padParams.add( pad8.set("Pad8", false ));
    
    kParams.add( k1.set("K1", 0.5, 0.0, 1.0 ));
    kParams.add( k2.set("K2", 0.5, 0.0, 1.0 ));
    kParams.add( k3.set("K3", 0.5, 0.0, 1.0 ));
    kParams.add( k4.set("K4", 0.5, 0.0, 1.0 ));
    kParams.add( k5.set("K5", 0.5, 0.0, 1.0 ));
    kParams.add( k6.set("K6", 0.5, 0.0, 1.0 ));
    kParams.add( k7.set("K7", 0.5, 0.0, 1.0 ));
    kParams.add( k8.set("K8", 0.5, 0.0, 1.0 ));
    
    mMidiParams.connect(0, true);
    mMidiParams.add(padParams);
    mMidiParams.add(kParams);
    
    // you could also add individually //
//    mMidiParams.add( k1 );
//    mMidiParams.add( k2 );
//    mMidiParams.add( k3 );
    mMidiParams.setPosition( ofGetWidth()-320, 20 );
    
    // to connect incoming MIDI to ofParameters passed into mMidiParams,
    // click on the parameter listed in the mMidiParams gui and
    // the next MIDI message will be used for mapping to that parameter.
    
    mMidiParams.load("example-midi-params.xml");
}

//--------------------------------------------------------------
void ofApp::update() {
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    float tx = 0.f;
    float ty = 0.f;
    ofPushMatrix(); {
        ofTranslate( 30, 30 );
        for( int i = 0; i < padParams.size(); i++ ) {
            if( i % 4 == 0 && i > 0 ) {
                tx = 0;
                ty += 150 + 10.f;
            }
            
            ofRectangle trect( tx, ty, 150, 150 );
            
            ofSetColor( 170 );
            ofDrawRectangle( trect );
            
            ofSetColor(60);
            ofDrawRectangle( trect.x+3, trect.y+3, trect.width-6, trect.height-6 );
            
            if( padParams.getBool(i)) {
                ofSetColor( 220, 50, 80 );
                ofDrawRectangle( trect.x+3, trect.y+3, trect.width-6, trect.height-6 );
            }
            ofSetColor( 100 );
            ofNoFill();
            ofDrawRectangle( trect );
            ofFill();
            
            tx += trect.width + 10.f;
            
        }
        
        ofTranslate( 75, 150 );
        ty += 150;
        tx =0;
        for( int i = 0; i < kParams.size(); i++ ) {
            if( i % 4 == 0 && i > 0 ) {
                tx = 0;
                ty += 150 + 10.f;
            }
            
            ofPushMatrix(); {
                ofTranslate( tx, ty );
                
                ofSetColor( 170 );
                ofDrawCircle(0,0,56);
                
                ofSetColor( 60 );
                ofDrawCircle(0,0,50);
                
                ofSetColor( 220 );
                ofPushMatrix(); {
                    ofRotateDeg( ofMap( kParams.getFloat(i), 0.0, 1.0, -140, 140 ));
                    ofDrawRectangle(-2, -50, 4, 50 );
                } ofPopMatrix();
                
                ofSetColor( 100 );
                ofNoFill();
                ofDrawCircle( 0, 0, 56 );
                ofFill();
                
            } ofPopMatrix();
            
            
            
            tx += 150 + 10.f;
            
        }
    } ofPopMatrix();
    
    mMidiParams.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
