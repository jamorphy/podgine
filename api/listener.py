import os
from twitchio.ext import commands
from dotenv import load_dotenv
from get_script import generate_podcast_script  # Import from your original file

load_dotenv()

class Bot(commands.Bot):
    def __init__(self):
        super().__init__(
            token=os.getenv('TWITCH_TOKEN'),
            prefix='!',
            initial_channels=['#jaajaa2001']  # Replace with your channel name
        )

    async def event_ready(self):
        print(f'Logged in as | {self.nick}')
        print(f'User id is | {self.user_id}')

    @commands.command()
    async def topic(self, ctx):
        # Get the message content after "!topic"
        topic = ctx.message.content[7:].strip()  # Remove "!topic " from the start
        
        if not topic:
            print("Error: Empty topic received")
            return

        print(f"\nNew topic received: {topic}")
        print("Generating podcast...")
        
        try:
            # Call your generate_podcast_script function
            output_file = generate_podcast_script(topic)
            print(f"Podcast generated successfully!")
            print(f"Saved to: {output_file}")
        except Exception as e:
            print(f"Error generating podcast: {str(e)}")

bot = Bot()
bot.run()
