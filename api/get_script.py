import json
import os
from datetime import datetime
from openai import OpenAI
from elevenlabs.client import ElevenLabs

from dotenv import load_dotenv
load_dotenv()

# Configuration
openai_client = OpenAI(api_key=os.getenv('OPENAI_API_KEY'))
eleven_client = ElevenLabs(api_key=os.getenv('ELEVENLABS_API_KEY'))

def generate_podcast_script(topic):
    output_dir = f"generated/podcast_{int(datetime.now().timestamp())}"
    os.makedirs(output_dir, exist_ok=True)
    
    system_prompt = """You are a JSON generator. Output must be valid JSON following this exact structure:
    {
        "topic": the given topic,
        "dialogue": [
            {
                "character": "buu_guy",
                "text": "statement here"
            },
            {
                "character": "kermit_da_frog", 
                "text": "response here"
            }
        ]
    }
    Rules:
    - Characters must alternate between "buu_guy" and "kermit_da_frog"
    - Start with buu_guy
    - Each text entry should be 1-3 sentences
    """

    response = openai_client.chat.completions.create(
        model="gpt-4o",
        messages=[
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": f"Generate a detailed, lengthy, debate-y and argumentative, conversation about the following topic: {topic}"}
        ]
    )
    
    # Parse the JSON response
    conversation = json.loads(response.choices[0].message.content)
    
    # Generate audio for each line of dialogue
    for i, entry in enumerate(conversation['dialogue']):
        # Select voice ID based on character
        voice_id = "HtU5TEQAgMkBMEhgdegX" if entry['character'] == "buu_guy" else "29vD33N1CtxCmqQRPOHJ"
        
        # Generate audio
        audio_stream = eleven_client.text_to_speech.convert(
            text=entry['text'],
            voice_id=voice_id,
            model_id="eleven_multilingual_v2"
        )
        
        # Save audio file
        audio_filename = f"{entry['character']}_{i}.mp3"
        audio_path = os.path.join(output_dir, audio_filename)
        with open(audio_path, 'wb') as f:
            audio_data = b''.join(chunk for chunk in audio_stream if isinstance(chunk, bytes))
            f.write(audio_data)
        
        # Add audio file path to the dialogue entry
        entry['audio_file'] = f"api/{output_dir}/{audio_filename}"
    
    # Save complete script with audio file paths
    script_file = f"{output_dir}/script.json"
    with open(script_file, 'w') as f:
        json.dump({
            **conversation,
            "timestamp": datetime.now().isoformat()
        }, f, indent=2)
    
    return script_file

if __name__ == "__main__":
    topic = "who is the greatest counter strike player of all time"
    output_file = generate_podcast_script(topic)
    print(f"Generated script saved to: {output_file}")
