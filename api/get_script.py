import json
import os
from datetime import datetime
from anthropic import Anthropic
from elevenlabs.client import ElevenLabs

from dotenv import load_dotenv
load_dotenv()

# Configuration
anthropic_client = Anthropic(api_key=os.getenv('ANTHROPIC_API_KEY'))
eleven_client = ElevenLabs(api_key=os.getenv('ELEVENLABS_API_KEY'))

def generate_podcast_script(topic):
    output_dir = f"generated/podcast_{int(datetime.now().timestamp())}"
    os.makedirs(output_dir, exist_ok=True)

    # TODO; read from manifest file from engine 
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
    try:
        with open("../engine_pipe", "w") as pipe:
            pipe.write(script_file + "\n")
    except:
        print("Failed to write to engine pipe")
    
    return script_file

if __name__ == "__main__":
    topic = "how can we verify if the colors that one human sees are the exact same colors that another human sees? dn might see the color blue, and kermit might see the color blue, but in dn's world blue is actually red in kermits world... interesting conversation"
    output_file = generate_podcast_script(topic)
    print(f"Generated script saved to: {output_file}")
