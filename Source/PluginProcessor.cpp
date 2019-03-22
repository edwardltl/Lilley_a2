/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Lilley_a2AudioProcessor::Lilley_a2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    currentSampleRate = 0.0f;
    currentAngle = 0.0f;
    angleDelta = 0.0f;
    sinFreq = 0.0f;
}

Lilley_a2AudioProcessor::~Lilley_a2AudioProcessor()
{
}

//==============================================================================
const String Lilley_a2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Lilley_a2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Lilley_a2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Lilley_a2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Lilley_a2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Lilley_a2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Lilley_a2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Lilley_a2AudioProcessor::setCurrentProgram (int index)
{
}

const String Lilley_a2AudioProcessor::getProgramName (int index)
{
    return {};
}

void Lilley_a2AudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Lilley_a2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    currentSampleRate = sampleRate;
    sinFreq = 1.0f;
    updateAngleDelta();
    
    String message;
    message << "Preparing to play..." << newLine;
    message << "My sample rate is " << currentSampleRate << newLine;
    message << "Buffer size is " << samplesPerBlock << newLine;
    Logger::getCurrentLogger()->writeToLog(message);
}

void Lilley_a2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Lilley_a2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Lilley_a2AudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //set up another copy of input buffer so we can keep our original
    AudioBuffer<float> wetBuffer(totalNumInputChannels, buffer.getNumSamples());
    wetBuffer.makeCopyOf(buffer);
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    // we are going through each channel of audio that is available
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        auto* wetData = wetBuffer.getWritePointer(channel);
        // we are getting an audio block of size 512
        
        // ..do something to the data...
        for (int sample= 0; sample < buffer.getNumSamples(); ++sample)
        {
            // generating a set of random values to modulate input amplitude
            /*
            float modulator = random.nextFloat() * 0.25f - 0.125f;
            wetData[sample] = wetData[sample] * modulator; // scaling & offsetting
            */
            
            // calculate value to put into buffer
            auto currentSinSample = (float) std::sin(currentAngle);
            currentAngle += angleDelta; // currentAngle = currentAngle + angleDelta
            // sine wave modulation
           // float deviations = random.nextFloat() * 0.1f - 0.05f;
            
            wetData[sample] = wetData[sample] * currentSinSample;
            
            /*
            // HARD clipping
            // auto shapedSample = jlimit((float) -0.1, (float) 0.1, wetData[sample]);
            
            // SOFT clip with hypobolic tangent
            // auto shapedSample = (32767 >> 1) * (float) std::atan(16 * wetData[sample] / 32767);
            */
            
            
            // crush those bits!
            float quantum = powf (1.5f, 8);
            
            auto shapedSample = floor(wetData[sample] * quantum) / quantum;
            wetData[sample] = shapedSample;
            
            // add original dry signal with processed signal into our output buffer (aka input buffer)
            channelData[sample] = channelData[sample] * 0.0f + wetData[sample] * 0.5;
        }
    }
}

//==============================================================================
bool Lilley_a2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Lilley_a2AudioProcessor::createEditor()
{
    return new Lilley_a2AudioProcessorEditor (*this);
}

//==============================================================================
void Lilley_a2AudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Lilley_a2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// user functions

void Lilley_a2AudioProcessor::updateAngleDelta()
{
    // calculate number of cycles that will need to complete for each output sample
    auto cyclesPerSample = sinFreq / currentSampleRate;
    // multiply by the length of a whole sine wave cycle
    angleDelta = cyclesPerSample * MathConstants<float>::twoPi;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Lilley_a2AudioProcessor();
}
