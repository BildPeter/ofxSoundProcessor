#include "ofApp.h"

//--------------------------------------------------------------

void ofApp::setup(){
    sampleRate = 44100;

    recorder.init( 512, sampleRate );
    recorder.startGrabbingSound();
    recorder.startRecording();
    recorder.stopPlaying();

    analyser.setup( recorder.getSampleRate(),
                    recorder.getBufferSizeCurrent() * 2,
                    recorder.getBufferSizeCurrent(),
                    recorder.getBufferSizeCurrent() / 2,
                    100 ); // call before ofSoundStreamSetup()
    analyser.setOnsetTimeSteps( 5 );
    analyser.setOnsetThreshold(0.015);

    std::cout << "Octave Analyser - Nr of averages: " << analyser.getOctaveAverages().size() << "\n";

    ofxMaxiSettings::setup( recorder.getSampleRate(), 1, recorder.getBufferSizeCurrent() );
    ofSoundStreamSetup( 1, 1, this, recorder.getSampleRate(), recorder.getBufferSizeCurrent(), 1 );
    maxiPlayer.load( ofToDataPath( "Geilo.wav" ) );

    playMaxi = true;

    startGrabbingSound.addListener(this, &ofApp::startGrb );
    startRecording.addListener(this, &ofApp::startRec );
    stopGrabbingSound.addListener(this, &ofApp::stopGrb );
    stopRecording.addListener(this, &ofApp::stopRec );
    startPlaying.addListener(this, &ofApp::startPlay );
    stopPlaying.addListener(this, &ofApp::stopPlay );
    restartPlaying.addListener(this, &ofApp::restartPlay );
    resetBuffer.addListener(this, &ofApp::resetBuff );
    resetAllMinMax.addListener( this, &ofApp::resetMinMax );
    loopPlayback.addListener( this, &ofApp::loopPlay );
    noLoopPlayback.addListener( this, &ofApp::noLoopPlay );

    gui.setup( "SoundAnalyser" );
    gui.add( magnitude.setup( "Strength", 5, 0, 10 ) );
    gui.add( startGrabbingSound.setup( "startGrabbingSound" ) );
    gui.add( startRecording.setup( "startRecording" ) );
    gui.add( stopGrabbingSound.setup( "stopGrabbingSound" ) );
    gui.add( stopRecording.setup( "stopRecording" ) );
    gui.add( startPlaying.setup( "startPlaying" ) );
    gui.add( stopPlaying.setup( "stopPlaying" ) );
    gui.add( loopPlayback.setup( "loopPlayback" ) );
    gui.add( noLoopPlayback.setup( "noLoopPlayback" ) );
    gui.add( restartPlaying.setup( "restartPlaying" ) );
    gui.add( resetBuffer.setup( "resetBuffer" ) );
    gui.add( resetAllMinMax.setup( "resetAllMinMax" ) );
}

//--------------------------------------------------------------
void ofApp::update(){
    analyser.update( recorder.getBufferCurrent() );
}

void ofApp::checkOctaves() {
    vector< float > averages = analyser.getOctaveAverages();
    //ofLog() << "averages count: " << averages.size();
    for( int i = 20; i < averages.size(); i++ ) {
        float value = averages.at(i);
        if( value > 2.0 ) {
            //ofLog() << i << " - " << averages.at(i);
            harmonicsCount += 1.0;
//            ofLog() << "harmonicness: " << (harmonicsCount / ofGetFrameNum() );
        }
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackgroundGradient( ofColor::gray, ofColor::black );

    drawOnset();
    drawBuffer();
    drawFFT( magnitude );
    drawOctave( magnitude);

    drawInfo();

    gui.draw();
}

void ofApp::drawInfo()
{
    ofDrawBitmapStringHighlight( "BufferSize: " + ofToString( recorder.getBufferRecorded().size() ) , 100, 100 );
    ofDrawBitmapStringHighlight( "Time: " + ofToString( ofGetElapsedTimeMillis() / 1000 ) , 100, 150 );
    ofDrawBitmapStringHighlight( "RMS: " + ofToString(          analyser.getRMS() ), 100, 200 );
    ofDrawBitmapStringHighlight( "MaxRMS: " + ofToString(       analyser.getMaxRMS() ), 100, 220 );
    ofDrawBitmapStringHighlight( "AMP: " + ofToString(          analyser.getAmplitude() ), 100, 240 );
    ofDrawBitmapStringHighlight( "MaxAMP: " + ofToString(       analyser.getMaxAmplitude() ), 100, 260 );
    ofDrawBitmapStringHighlight( "Onset: " + ofToString(        analyser.getIfOnset() ), 100, 280 );
    ofDrawBitmapStringHighlight( "FFT Centroid: " + ofToString( analyser.getFFTspectralCentroid() ), 100, 300 );
    ofDrawBitmapStringHighlight( "FFT Centroid Min: " + ofToString( analyser.getFFTspectralCentroidMin() ), 100, 320 );
    ofDrawBitmapStringHighlight( "FFT Centroid Max: " + ofToString( analyser.getFFTspectralCentroidMax() ), 100, 340 );
    ofDrawBitmapStringHighlight( "FFT Flatness: " + ofToString( analyser.getFFTspectralFlatness() ), 100, 360 );
    ofDrawBitmapStringHighlight( "FFT Flatness Min: " + ofToString( analyser.getFFTspectralFlatnessMin() ), 100, 380 );
    ofDrawBitmapStringHighlight( "FFT Flatness Max: " + ofToString( analyser.getFFTspectralFlatnessMax() ), 100, 400 );
}

void ofApp::drawOctave( float magnScale )
{
    ofSetColor( ofColor::blue );
    int xStep = ofGetWidth() / analyser.getOctaveAverages().size();
    int yLine = ofGetHeight() * 3 / 4;
    float height = 0.0;
    int i = 0;
    for( auto &_octaveAverage : analyser.getOctaveAverages() )
    {
        height = _octaveAverage * 20 * 3 / 2;
        ofDrawRectangle( xStep * i++, yLine, xStep,  -( height * magnScale ) );
    }
}

void ofApp::drawFFT( float magnScale )
{
    int i  = 0;
    int xStep = ofGetWidth() / analyser.getFFTSpectralMagnitudes().size();
    int yLine = ofGetHeight() / 3;

    ofSetColor(0);
    ofSetLineWidth(2);
    ofDrawLine( 0, yLine, ofGetWidth(), yLine );

    ofSetColor( ofColor::red, 255 );
    for( auto &magnMax : analyser.getFFTSpectralMagnitudesMaximums() ) {
        ofDrawRectangle( xStep * i++, yLine, xStep,  -( magnMax * magnScale ) );
    }

    ofSetColor( ofColor::lightGreen, 255 );
    i = 0;
    for( auto &magn : analyser.getFFTSpectralMagnitudes() ) {
        ofDrawRectangle( xStep * i++, yLine, xStep,  -( magn * magnScale ) );
    }
}

void ofApp::drawOnset()
{
    if ( analyser.getIfOnset() )
    {
        ofSetColor( ofColor::wheat, 255 );
        ofDrawRectangle( 0, 0, ofGetWidth(), 400 );

        checkOctaves();
    }
}

void ofApp::drawBuffer()
{
    ofPushStyle();
    float y1 = ofGetHeight() * 0.5;
    ofSetColor(0);
    ofSetLineWidth(2);
    ofDrawLine(0, y1, ofGetWidth(), y1);

    // --- Drawing Buffer TimeLine
    ofSetColor( ofColor::lightBlue );
    ofSetLineWidth(1);
    for( int i=0; i < recorder.getBufferSizeCurrent(); i++){
        float p = i / (float)( recorder.getBufferSizeCurrent() - 1 );
        float x = p * ofGetWidth();
        float y2 = y1 + recorder.getBufferCurrent()[i] * 800;

        ofDrawLine(x, y1, x, y2);
    }
    ofPopStyle();
}
//--------------------------------------------------------------

void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    if ( useMic ) {
        recorder.grabAudioBuffer( input );
    }
}

//--------------------------------------------------------------
void ofApp::audioOut( float *output, int bufferSize, int nChannels )
{
    if ( playMaxi  && recorder.isGrabbing() ) // ensure data isn't read, before MaxiSample is loaded
    {
        for (int i = 0; i < bufferSize; i++)
        {
            output[ i * nChannels ] = maxiPlayer.play();
        }
        if ( !useMic ) {
            recorder.grabAudioBuffer( output );
        }
    }
    else if ( recorder.isPlaying() )
    {
        recorder.outputAudioBuffer( output );
    }
    else
    {
        for (int i = 0; i < bufferSize; i++)
        {
            output[ i * nChannels ] = 0.0;
        }
    }
}