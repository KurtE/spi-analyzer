#include "SpiExAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "SpiExAnalyzer.h"
#include "SpiExAnalyzerSettings.h"
#include <iostream>
#include <sstream>

#pragma warning(disable: 4996) //warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.

SpiExAnalyzerResults::SpiExAnalyzerResults( SpiExAnalyzer* analyzer, SpiExAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

SpiExAnalyzerResults::~SpiExAnalyzerResults()
{
}

void SpiExAnalyzerResults::GenerateBubbleText(U64 frame_index, Channel& channel, DisplayBase display_base)  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	Frame frame = GetFrame(frame_index);

	if ((frame.mFlags & SPI_ERROR_FLAG) == 0)
	{
		if (channel == mSettings->mMosiChannel)
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString(frame.mData1, display_base, mSettings->mBitsPerTransfer, number_str, 128);
			AddResultString(number_str);
		}
		else if (channel == mSettings->mMisoChannel)
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString(frame.mData2 & 0x7fffffffffffffffull, display_base, mSettings->mBitsPerTransfer, number_str, 128);
			AddResultString(number_str);
		}
		else if (channel == mSettings->mDCChannel)
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString(frame.mData2 >> 63, display_base, 1, number_str, 128);
			AddResultString(number_str);
		}
	}
	else
	{
			AddResultString( "Error" );
			AddResultString( "Settings mismatch" );
			AddResultString( "The initial (idle) state of the CLK line does not match the settings." );
	}
}

void SpiExAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 /*export_type_user_id*/ )
{
	//export_type_user_id is only important if we have more than one export type.


	std::stringstream ss;
	void* f = AnalyzerHelpers::StartFile( file );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	ss << "Time [s],Packet ID,MOSI,MISO,DC" << std::endl;

	bool mosi_used = true;
	bool miso_used = true;
	bool dc_used = true;

	if( mSettings->mMosiChannel == UNDEFINED_CHANNEL )
		mosi_used = false;

	if( mSettings->mMisoChannel == UNDEFINED_CHANNEL )
		miso_used = false;

	if (mSettings->mDCChannel == UNDEFINED_CHANNEL)
		dc_used = false;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );

		if( ( frame.mFlags & SPI_ERROR_FLAG ) != 0 )
			continue;
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char mosi_str[128] = "";
		if( mosi_used == true )
			AnalyzerHelpers::GetNumberString( frame.mData1, display_base, mSettings->mBitsPerTransfer, mosi_str, 128 );

		char miso_str[128] = "";
		if( miso_used == true )
			AnalyzerHelpers::GetNumberString( frame.mData2 & 0x7fffffffffffffffull, display_base, mSettings->mBitsPerTransfer, miso_str, 128 );

		char dc_str[128] = "";
		if (dc_used == true)
			AnalyzerHelpers::GetNumberString( frame.mData2 >> 63, display_base, 1, miso_str, 128);

		U64 packet_id = GetPacketContainingFrameSequential( i ); 
		if( packet_id != INVALID_RESULT_INDEX )
			ss << time_str << "," << packet_id << "," << mosi_str << "," << miso_str << "," << dc_str << std::endl;
		else
			ss << time_str << ",," << mosi_str << "," << miso_str << "," << dc_str << std::endl;  //it's ok for a frame not to be included in a packet.
	
		AnalyzerHelpers::AppendToFile( (U8*)ss.str().c_str(), (U32)ss.str().length(), f );
		ss.str( std::string() );
							
		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			AnalyzerHelpers::EndFile( f );
			return;
		}
	}

	UpdateExportProgressAndCheckForCancel( num_frames, num_frames );
	AnalyzerHelpers::EndFile( f );
}

void SpiExAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
    ClearTabularText();
	Frame frame = GetFrame( frame_index );

	bool mosi_used = true;
	bool miso_used = true;
	bool dc_used = true;

	if( mSettings->mMosiChannel == UNDEFINED_CHANNEL )
		mosi_used = false;

	if( mSettings->mMisoChannel == UNDEFINED_CHANNEL )
		miso_used = false;

	if (mSettings->mDCChannel == UNDEFINED_CHANNEL)
		dc_used = false;

	char mosi_str[128];
    char miso_str[128];
	char dc_str[128];

    std::stringstream ss;

    if( ( frame.mFlags & SPI_ERROR_FLAG ) == 0 )
    {
        if( mosi_used == true )
            AnalyzerHelpers::GetNumberString( frame.mData1, display_base, mSettings->mBitsPerTransfer, mosi_str, 128 );
        if( miso_used == true )
            AnalyzerHelpers::GetNumberString( frame.mData2 & 0x7fffffffffffffffull, display_base, mSettings->mBitsPerTransfer, miso_str, 128 );
		if (dc_used == true)
			AnalyzerHelpers::GetNumberString(frame.mData2 >> 63, display_base, 1, dc_str, 128);

		bool field_output = false;
		if (mosi_used == true)
		{
			ss << "MOSI: " << mosi_str;
			field_output = true;
		}
		if (miso_used == true)
		{
			if (field_output) ss << ";  MISO: " << miso_str;
			else ss << "MISO: " << miso_str;
			field_output = true;
		}
		if (dc_used == true)
		{
			if (field_output) ss << ";  DC: " << dc_str;
			else ss << "DC: " << dc_str;
		}
    }
    else
    {
        ss << "The initial (idle) state of the CLK line does not match the settings.";
    }

	AddTabularText( ss.str().c_str() );
}

void SpiExAnalyzerResults::GeneratePacketTabularText( U64 /*packet_id*/, DisplayBase /*display_base*/ )  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void SpiExAnalyzerResults::GenerateTransactionTabularText( U64 /*transaction_id*/, DisplayBase /*display_base*/ )  //unrefereced vars commented out to remove warnings.
{
	ClearResultStrings();
	AddResultString( "not supported" );
}
