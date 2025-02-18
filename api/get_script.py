import json
import os
import requests
from datetime import datetime
from anthropic import Anthropic
from elevenlabs.client import ElevenLabs

from dotenv import load_dotenv
load_dotenv()

# Configuration
anthropic_client = Anthropic(api_key=os.getenv('ANTHROPIC_API_KEY'))
eleven_client = ElevenLabs(api_key=os.getenv('ELEVENLABS_API_KEY'))

# Toggle for local TTS
USE_LOCAL_TTS = True
LOCAL_TTS_URL = 'http://192.168.254.11:5000/tts'

def generate_audio(text, voice_id, output_path):
    """Helper function to generate audio using either ElevenLabs or local TTS"""
    if USE_LOCAL_TTS:
        response = requests.post(
            LOCAL_TTS_URL,
            json={
                "text": text,
                "voice_id": voice_id
            }
        )
        if response.status_code != 200:
            raise Exception(f"Local TTS error: {response.text}")
        audio_data = response.content
    else:
        audio_stream = eleven_client.text_to_speech.convert(
            text=text,
            voice_id=voice_id,
            model_id="eleven_multilingual_v2"
        )
        audio_data = b''.join(chunk for chunk in audio_stream if isinstance(chunk, bytes))
    
    # Save the audio file
    with open(output_path, 'wb') as f:
        f.write(audio_data)

def generate_podcast_script(topic):
    output_dir = f"generated/podcast_{int(datetime.now().timestamp())}"
    os.makedirs(output_dir, exist_ok=True)

    system_prompt = """You are a JSON generator. Output must be valid JSON following this exact structure:
    {
        "topic": the given topic,
        "dialogue": [
            {
                "character": "jaja",
                "text": "statement here"
            },
            {
                "character": "kermit", 
                "text": "response here"
            },
            {
                "character": "dn", 
                "text": "rebuttal here"
            },
            .... continue the conversation for multiple turns
        ]
    }
    Rules:
    - Characters must alternate between "jaja", "kermit", "dn"
    - Start with jaja
    - Each text entry should be 1-2 sentences
    """

    response = anthropic_client.messages.create(
        model="claude-3-opus-20240229",
        max_tokens=2000,
        system=system_prompt,
        messages=[
            {
                "role": "user", 
                "content": f"Generate a detailed, lengthy, debate-y and argumentative, conversation about the following topic: {topic}."
            }
        ]
    )
    
    # Parse the JSON response
    conversation = json.loads(response.content[0].text)
    
    # Generate audio for each line of dialogue
    for i, entry in enumerate(conversation['dialogue']):
        # Select voice ID based on character
        cid = entry['character']
        if cid == "jaja":
            voice_id = "HtU5TEQAgMkBMEhgdegX"
        elif cid == "kermit":
            voice_id = "29vD33N1CtxCmqQRPOHJ"
        elif cid == "dn":
            voice_id = "Zlb1dXrM653N07WRdFW3"
        
        # Generate and save audio file
        audio_filename = f"{entry['character']}_{i}.mp3"
        audio_path = os.path.join(output_dir, audio_filename)
        
        try:
            generate_audio(entry['text'], voice_id, audio_path)
        except Exception as e:
            print(f"Error generating audio for entry {i}: {str(e)}")
            continue
        
        # Add audio file path to the dialogue entry
        entry['audio_file'] = f"api/{output_dir}/{audio_filename}"
    
    # Save complete script with audio file paths
    script_file = f"{output_dir}/script.json"
    with open(script_file, 'w') as f:
        json.dump({
            **conversation,
            "timestamp": datetime.now().isoformat()
        }, f, indent=2)

    try:
        with open("../engine_pipe", "w") as pipe:
            pipe.write(f'api/{script_file}' + "\n")
    except:
        print("Failed to write to engine pipe")
    
    return script_file

if __name__ == "__main__":
    # Print which TTS service we're using
    print(f"Using {'Local' if USE_LOCAL_TTS else 'ElevenLabs'} TTS service")
    
    topic = "best mma fighter of all time? How would they compare to a mongol warrior in hand to hand combat?"
    output_file = generate_podcast_script(topic)
    print(f"Generated script saved to: {output_file}")
