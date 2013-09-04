/*
 *  IKLine.cpp
 *  Paint
 *
 *  Created by Mario.Gonzalez on 6/21/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "RibbonPaint.h"
//#include "IKLine.h"
//#include "Segment.h"


#ifdef __RIBBON__PAINT__XCODE__
#import "MyCinderGLDrawing.h"
#define LOAD_RESOURCE(__X__)  [[MyCinderGLDrawing instance] loadResource: __X__ ]
#else
#define LOAD_RESOURCE(__X__) loadResource(__X__)
#endif

//#import "gl.h"
void RibbonPaint::reymentaSetup()
{
	mBackgroundColor = ColorAf( 0.0f, 0.0f, 0.0f, 0.0f );
	mDisplayCount = 0;
	mRenderY = 0;
	for (auto display : Display::getDisplays() )
	{
		mDisplayCount++;
		std::cout << "Width:" << display->getWidth() << "\n"; 
		std::cout << "Height:" << display->getHeight() << "\n"; 
		mRenderWidth = display->getWidth();
		mRenderHeight = display->getHeight();

		//mRenderX mRenderY mRenderWidth mRenderHeight
	}
	std::cout << "Display Count:" << mDisplayCount << "\n";
	if ( mDisplayCount == 1 )
	{
		mRenderX = mMainDisplayWidth / 2;
		mRenderWidth /= 2;
		mRenderHeight /= 2;
	}
	else
	{
		mRenderX = mMainDisplayWidth;
	}
	setWindowSize( mRenderWidth, mRenderHeight );
	
	WindowRef rWin = app::getWindow();
	rWin->setPos(mRenderX, mRenderY);
	rWin->setBorderless();
	rWin->setAlwaysOnTop();

	HWND hWnd = (HWND)rWin->getNative();

	HRESULT hr = S_OK;
	// Create and populate the Blur Behind structure
	DWM_BLURBEHIND bb = {0};

	// Enable Blur Behind and apply to the entire client area
	bb.dwFlags = DWM_BB_ENABLE;
	bb.fEnable = true;
	bb.hRgnBlur = NULL;

	// Apply Blur Behind
	hr = DwmEnableBlurBehindWindow(hWnd, &bb);
	if (SUCCEEDED(hr))
	{
		HRESULT hr = S_OK;

		// Set the margins, extending the bottom margin.
		MARGINS margins = {-1};

		// Extend the frame on the bottom of the client area.
		hr = DwmExtendFrameIntoClientArea(hWnd,&margins);
	}
	
	//setWindowPos( mMainDisplayWidth, 0 );
	//setFullScreen( ! isFullScreen() );
	receiver.setup( 10001 );
}
void RibbonPaint::quitProgram()
{
	quit();
}

void RibbonPaint::prepareSettings(Settings *settings) {
	settings->setWindowSize(1200, 800);
	settings->setFrameRate( 60 );
	settings->setTitle("RibbonPaint");
	//settings->setDisplay( ci::Display::getDisplays().at(1) );
}

void RibbonPaint::setup()
{
	reymentaSetup();

	ci::Rand::randSeed( 2 );
	_perlinNoise.setSeed( sin( time( NULL ) ) * 10);
	_perlinNoise.setOctaves( 4 );


	_alertTextDefaultDisplayTimeInFrames = 60 * 3;
	_mousePosition = getWindowCenter();
	_gravity = 0.0;
	_glitchSegment = false;
	_drawPins = false;

	// Behavior modification
	_alphaWhenDrawing = 0.11;
	_mouseChaseDamping = 0.2;

	_colorStateManager.setInitialState( new ColorModes::ColorModeRGB() );

	_colorMap['1'] = new ColorModes::ColorModeHSV();
	_colorMap['2'] = new ColorModes::ColorModeRGB();
	_colorMap['3'] = new ColorModes::ColorModeRGBInverse();
	_colorMap['4'] = new ColorModes::ColorModeRGB2();
	_colorMap['5'] = new ColorModes::ColorModeRGB2();
	_colorMap['6'] = new ColorModes::ColorModeGrayscale();
	_colorMap['7'] = new ColorModes::ColorModeAlphaBlend1();
	_colorMap['8'] = new ColorModes::ColorModeAlphaBlend2();
	_colorMap['9'] = new ColorModes::ColorModeAlphaBlend3();
	_additiveBlending = false;
	_useBezier = false;
	randomizeBrush();
}

#pragma mark Events

void RibbonPaint::mouseDown( ci::Vec2f position ){

	_oldAlphaWhenDrawing = _alphaWhenDrawing;
	//ci::gl::clear( _colorStateManager.getColorModel()->getBackgroundColor() );
}

void RibbonPaint::mouseDrag( ci::Vec2f position )
{

	// Not drawing lines, clear the frame
	/*if(_drawLines == false)
		ci::gl::clear( _colorStateManager.getColorModel()->getBackgroundColor() );*/

	_alphaWhenDrawing = _oldAlphaWhenDrawing;

	// start drawing lines
	_mousePosition = position;
	_drawLines = true;
}

void RibbonPaint::mouseMove( ci::Vec2f position )
{
	_mousePosition = position;
	//    std::cout << [[MyCinderGLDrawing instance] getWindowWidth] << std::endl;
}

void RibbonPaint::mouseUp( ci::Vec2f position ) {
	// Alpha when drawing becomes whatever is the highest (incase they put the mouse up before letting go of shift)
	_alphaWhenDrawing = ci::math<double>::max(_oldAlphaWhenDrawing, _alphaWhenDrawing);
	_oldAlphaWhenDrawing = _alphaWhenDrawing;

	//ci::gl::clear( _colorStateManager.getColorModel()->getBackgroundColor() );
	_drawLines = false;
}

void RibbonPaint::keyUp( ci::app::KeyEvent event )
{

}

void RibbonPaint::keyDown( ci::app::KeyEvent event )
{

	std::map<char, ColorModes::IColorMode*>::iterator itr = _colorMap.find( event.getChar() );
	if(itr != _colorMap.end() ) {
		_colorStateManager.changeState( itr->second );
	}

	char keyPressed = event.getChar();
	int colorMode;
	switch (keyPressed)
	{
		case 'r':
			randomizeBrush();
			break;
		case 'm':
			createBrush();
			break;
		case 'g':
			_glitchSegment = !_glitchSegment;
			break;
		case 'l':
			_useBezier = !_useBezier;
		case 'p':
			_drawPins = !_drawPins;
			break;
		case '1': // COLORMODE_HSV
		case '2': // COLORMODE_RGB
		case '3': // COLORMODE_RGBINVERSE
		case '4': // COLORMODE_RGB_B
		case '5': // COLORMODE_RGB_C
		case '6': // COLORMODE_GRAYSCALE
		case '7': // COLORMODE_ALPHABLEND_1
		case '8': // COLORMODE_ALPHABLEND_2
			colorMode = boost::lexical_cast<int>( keyPressed );
//			toggleAdditiveBlending( colorMode < COLORMODE_GRAYSCALE);
//			_colorMode = colorMode;
			break;
		default:
			std::cout << keyPressed << std::endl;
			break;
	}

	// slowdown or speed up how fast our object chases the mouse - clamp addition of speed (determined by ternary operator)
	if(event.getCode() == ci::app::KeyEvent::KEY_LEFT || event.getCode() == ci::app::KeyEvent::KEY_RIGHT) {
		if(event.getCode() == ci::app::KeyEvent::KEY_LEFT)
			_mouseChaseDamping -= 0.02;
		else if(event.getCode() == ci::app::KeyEvent::KEY_RIGHT)
			_mouseChaseDamping += 0.02;

		_mouseChaseDamping = ci::math<float>::clamp(_mouseChaseDamping, 0.02, 1.0);
	}

	if(event.getCode() == ci::app::KeyEvent::KEY_UP || event.getCode() == ci::app::KeyEvent::KEY_DOWN) {
		_alphaWhenDrawing = ci::math<double>::clamp(_alphaWhenDrawing + ((event.getCode() == ci::app::KeyEvent::KEY_UP) ? 0.005 : -0.005), 0.0, 1.0);
	}

	// exit fullscreen with escape, and toggle with F
	if ( event.getChar() == 'f' || (isFullScreen() && event.getCode() == ci::app::KeyEvent::KEY_ESCAPE) ) {
		//ci::gl::clear( _colorStateManager.getColorModel()->getBackgroundColor() );
		setFullScreen( !isFullScreen() );
	}
}

void RibbonPaint::saveOutBrushImageAndParameters()
{
	using namespace boost::posix_time;

	// Use boost to figure out the current time on local machine
	ptime now = second_clock::local_time();

	// The time is nigh!
	std::stringstream ss;
	ss << now.date().month() <<  now.date().day() << "_";
	std::string timeStamp = ss.str();
}


void RibbonPaint::randomizeBrush()
{
	std::vector<std::string> randomParamsList;
	randomParamsList.push_back("Parameters Used: [23,9,8,15,0.78,0.85,0.0]");
	randomParamsList.push_back("Parameters Used: [24,9.5,7,30,0.83,0.87,0.0]");
	randomParamsList.push_back("Parameters Used: [100,0,4.5,45,0.75,0.82,0.0]");
	randomParamsList.push_back("Parameters Used: [100,22.5,1.5,30,0.77,0.77,0.0]");
	randomParamsList.push_back("Parameters Used: [100,25,1.5,30,0.77,0.77,0.50,0.059999999999999921,0.0400000066]");
	randomParamsList.push_back("Parameters Used: [9,1.5,4.5,45,0.74,0.76,0.0]");
	randomParamsList.push_back("Parameters Used: [23,5,1.5,45,0.76,0.83,0.0]");
	randomParamsList.push_back("Parameters Used: [12,2.5,2.5,65,0.72,0.79,0.13]");
	randomParamsList.push_back("Parameters Used: [5,0,2,55,0.84,0.86,0.0]");
	randomParamsList.push_back("Parameters Used: [5,0,2,55,0.84,0.86,0.0]");
	randomParamsList.push_back("Parameters Used: [8,0,2.5,85,0.8,0.883,0.0]");
	randomParamsList.push_back("Parameters Used: [31,0,4.5,25,0.84,0.86,0.0]");
	randomParamsList.push_back("Parameters Used: [8,0,2,55,0.84,0.86,0.0]");

	int i = ci::Rand::randInt( randomParamsList.size() );
	std::string randomParams = randomParamsList[i];

	std::cout  << randomParams << std::endl;
	setBrushSettingsFromStringParameters( randomParams );
//	_colorMode = ci::Rand::randInt(COLORMODE_HSV, COLORMODE_ALPHABLEND_3);
//	toggleAdditiveBlending( _colorMode < COLORMODE_GRAYSCALE );
	createBrush();
}

void RibbonPaint::setBrushSettingsFromStringParameters( std::string currentLine )
{
	// grab all the values from the string by splitting on ","
	std::vector<std::string> values;
	boost::split(values, currentLine, boost::is_any_of(","));

	std::cout  << "Setting Strings from: " << currentLine << std::endl;
	if( values.size() != 7 && values.size() != 9) return; // more or less than 7, just exit file is no good
	for (int i = 0; i < values.size() ; i++)
	{
		std::vector<std::string> split;

		std::cout << values[i] << std::endl;
		// use boost::lexical_cast to convert the strings to the proper datatype
		switch (i)
		{
			case 0: // This one has the prefix above as part of its string
				boost::split(split, values[i], boost::is_any_of("["));
				_bristleCount = boost::lexical_cast<int>( split[1] );
				break;
			case 1:
				_brushRadius = boost::lexical_cast<float>( values[i] );
				break;
			case 2:
				_filamentSpacing = boost::lexical_cast<float>( values[i] );
				break;
			case 3:
				_filamentCount = boost::lexical_cast<float>( values[i] );
				break;
			case 4:
				_canvasFrictionMin = boost::lexical_cast<float>( values[i] );
				break;
			case 5:
				_canvasFrictionMax = boost::lexical_cast<float>( values[i] );
				break;
			case 6:
				split.clear();
				boost::split(split, values[i], boost::is_any_of("]"));

				if(split.size() > 0) {
					boost::split(split, values[i], boost::is_any_of("]"));
					_gravity = boost::lexical_cast<float>( split[0] );
				} else {
					_gravity = boost::lexical_cast<float>( values[i] );
				}
				break;
			case 7:
				_alphaWhenDrawing = boost::lexical_cast<double>( values[i] );
				break;
			case 8:
				boost::split(split, values[i], boost::is_any_of("]"));
				_mouseChaseDamping = boost::lexical_cast<float>( split[0] );
			default:
				break;
		};
	}
	// Everythign went great - lets recreate recreate the brush
	createBrush();
}

void RibbonPaint::createBrush()
{
	if(_lines.size() != 0)
		_lines.clear();

	if(_canvasFrictionMin >= _canvasFrictionMax)
		_canvasFrictionMin = _canvasFrictionMax;

	for (int i = 0; i < _bristleCount; i++)
	{
		float radius = ci::Rand::randFloat() * _brushRadius;
		float radian = ci::Rand::randFloat() * M_PI * 2;

		//linePointer
		linePointer line = linePointer(new IKLine());

		line->x = cosf(radian) * radius;
		line->y = sinf(radian) * radius;

		line->segmentLength = _filamentSpacing;
		line->segmentNum = _filamentCount;
		line->gravity = _gravity;
		line->friction = ci::Rand::randFloat(_canvasFrictionMin, _canvasFrictionMax);
		line->init();

		_lines.push_back(line);
	}
}

void RibbonPaint::update()
{
	while( receiver.hasWaitingMessages() ) {
		osc::Message m;
		receiver.getNextMessage( &m );

		console() << "New message received" << std::endl;
		console() << "Address: " << m.getAddress() << std::endl;
		console() << "Num Arg: " << m.getNumArgs() << std::endl;
		// check for mouse moved message
		if(m.getAddress() == "/mouse/position"){
			// both the arguments are int32's
			Vec2i pos = Vec2i( m.getArgAsInt32(0), m.getArgAsInt32(1));
			_mousePosition = pos;
			if ( m.getArgAsInt32(2) == 1 )
			{
				mMouseDown = true;
			}
			else
			{
				mMouseDown = false;
			}
			if ( mMouseDown )
			{
				_oldAlphaWhenDrawing = _alphaWhenDrawing;

				// Not drawing lines, clear the frame
				/*if(_drawLines == false)
					ci::gl::clear( _colorStateManager.getColorModel()->getBackgroundColor() );*/

				// start drawing lines
				_drawLines = true;
			}
		}
		// check for mouse button message
		else if(m.getAddress() == "/mouse/button"){
			_oldAlphaWhenDrawing = _alphaWhenDrawing;
			if ( m.getArgAsInt32(2) == 1 )
			{
				mMouseDown = true;
			}
			else
			{
				mMouseDown = false;
			}
		}
		else if(m.getAddress() == "/ribbon/randomizebrush"){
			randomizeBrush();
			/* TODO createBrush();
			_glitchSegment = !_glitchSegment;
			_useBezier = !_useBezier;*/
		}
		else if(m.getAddress() == "/window/position"){
			// window position
			setWindowPos(m.getArgAsInt32(0), m.getArgAsInt32(1));
		}
		else if(m.getAddress() == "/window/setfullscreen"){
			// fullscreen
			setFullScreen( ! isFullScreen() );
		}		
		else if(m.getAddress() == "/quit"){
			quitProgram();
		}		
		else{
			// unrecognized message
			//cout << "not recognized:" << m.getAddress() << endl;

		}

	}

	float nX = (mRenderWidth/2) * 0.01f;
	float nY = (mRenderHeight/2) * 0.01f;
	float nZ = getElapsedFrames() * 0.003f;

	float mNoise = _perlinNoise.noise( nX, nY, nZ );

	float angle = mNoise * M_PI;


	//    std::cout << _mousePosition << std::endl;
	ci::Vec2f mousePosition;
	bool doNoise = false;

	if (doNoise)
	{
		float yRange = getElapsedFrames() * 0.4;
		ci::Vec2f noiseVector( cosf( angle ) * 100 + (_mousePosition.x*0.1) , sinf( angle ) * yRange + (_mousePosition.y*0.1));
		//		mousePosition =
	} else {
		mousePosition = _mousePosition;
	}



	static float i;
	for( std::vector<linePointer>::iterator p = _lines.begin(); p != _lines.end(); ++p) {

		if(doNoise)
		{
			float localNoise = _perlinNoise.fBm( i*0.01f );
			localNoise*=5;

			ci::Vec2f localPos = mousePosition;
			localPos.x += cosf(localNoise) * 5;
			localPos.y += sinf(localNoise) * 5;
			(*p)->nextFrame(localPos, _mouseChaseDamping);

			i++;
		}
		else
		{
			(*p)->nextFrame(mousePosition, _mouseChaseDamping);
		}
	}
}

void RibbonPaint::draw()
{
	//gl::clear( mBackgroundColor );
	_alphaWhenDrawingFloat = _alphaWhenDrawing;
	ci::gl::setMatricesWindow( ci::Vec2i(mRenderWidth, mRenderHeight ), true);

	// Not drawing, clear screen
	if( _drawLines == false ) {
		//ci::gl::clear( _colorStateManager.getColorModel()->getBackgroundColor() );
	} else { // overdraw to clear a little
		//		ColorA stringColor = (_colorMode > COLORMODE_GRAYSCALE) ? ColorA(1.0, 1.0, 1.0, 1.0) : ColorA(0.0, 0.0, 0.0, 1.0);
		//		glColor4f( stringColor.r, stringColor.g, stringColor.b, 0.01 );
		//		gl::drawSolidRect(Rectf(0, 0, getWindowWidth(), getWindowHeight() ) );
	}

	glEnableClientState(GL_VERTEX_ARRAY);
		for( std::vector<linePointer>::iterator p = _lines.begin(); p != _lines.end(); ++p) {
			drawIKLine( *p );
		}
	glDisableClientState(GL_VERTEX_ARRAY);
}


void RibbonPaint::drawIKLine( linePointer& lineToDraw ) {
	ci::Vec2f start = ci::Vec2f::zero();
	start.x = lineToDraw->x + lineToDraw->_segments[0]->x;
	start.y = lineToDraw->y + lineToDraw->_segments[0]->y;

	double alpha;

	// If drawing lines, h
	if(_drawLines) {
//		if(_colorMode <= COLORMODE_GRAYSCALE) alpha = _alphaWhenDrawing;
//		else alpha = _alphaWhenDrawing*.55;
	} else {
		alpha = 1.0;
	}

	for (int i = 0; i < lineToDraw->segmentNum - 2; i++) {
		ci::Vec2f end = ci::Vec2f::zero();
		end.x = (lineToDraw->x + lineToDraw->_segments[i]->x + lineToDraw->x + lineToDraw->_segments[i+1]->x ) * 0.5;
		end.y = (lineToDraw->y + lineToDraw->_segments[i]->y + lineToDraw->y + lineToDraw->_segments[i+1]->y ) * 0.5;

		ci::Vec2f control = ci::Vec2f::zero();
		control.x = lineToDraw->x + lineToDraw->_segments[i]->x;
		control.y = lineToDraw->y + lineToDraw->_segments[i]->y;


		// Get the color based on distance in array from line
		double nonInverseI = ( (double)i / (double)(lineToDraw->segmentNum-2) );
		double inverseI = 1.0f - nonInverseI;

		//getColorMode( inverseI, alpha);
		_colorStateManager.getColorModel()->setColor( inverseI );
		glLineWidth(2);

		Segment segment = *(lineToDraw->_segments[i]);
		if(_useBezier) {
			if(_drawPins) {
				ci::Vec2f pin = segment.getPin();
				ci::gl::drawSolidCircle(ci::Vec2f(lineToDraw->x, lineToDraw->y) + pin, 2, 6);

				glEnableClientState(GL_VERTEX_ARRAY);
				drawBezier(start, control, end, 5);
				glDisableClientState(GL_VERTEX_ARRAY);
			} else {
				drawBezier(start, control, end, 10);
			}
		} else { // draw straight lines
			if(_drawPins) {
				ci::Vec2f pin = segment.getPin();
				ci::gl::drawSolidCircle(ci::Vec2f(lineToDraw->x, lineToDraw->y) + pin, 2, 6);
			}

			glEnableClientState(GL_VERTEX_ARRAY);
			ci::Vec2f vertices[2];
			vertices[0] = start;
			vertices[1] = end;

			//glVertexPointer(2, GL_FLOAT, 0, vertices);
			//glDrawArrays(GL_LINES, 0, 2);
			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glDrawArrays(GL_LINE_STRIP, 0, 2);
			glDisableClientState(GL_VERTEX_ARRAY);

		}

		start = end;
	}
}

inline void RibbonPaint::drawBezier(ci::Vec2f origin, ci::Vec2f control, ci::Vec2f destination, const int segments) {

	//vector<ci::Vec2f> vertices;
	//float t = 0.0;
	//for(int i = 0; i <= segments-1; i++) {
	//	float x = pow(1.0f - t, 2) * origin.x + 2.0f * (1.0f - t) * t * control.x + t * t * destination.x;
	//	float y = pow(1.0f - t, 2) * origin.y + 2.0f * (1.0f - t) * t * control.y + t * t * destination.y;
	//	vertices.push_back(ci::Vec2f(x, y));
	//	//vertices[i] = ci::Vec2f(x, y);
	//	t += 1.0f / segments;
	//}
	//vertices[segments] = destination;


}



	CINDER_APP_BASIC( RibbonPaint, ci::app::RendererGl )


