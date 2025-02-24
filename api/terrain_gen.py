import jax
import jax.numpy as jnp
from jax import random, grad, value_and_grad
import optax

# Model constants
GRID_SIZE = 128
TOTAL_SIZE = GRID_SIZE * GRID_SIZE
INPUT_DIM = 2  # (x, z) coordinates
HIDDEN_DIM = 64
OUTPUT_DIM = 1  # Single height value

# Model definition (same as before)
def init_params(key):
    k1, k2, k3 = random.split(key, 3)
    w1 = random.normal(k1, (INPUT_DIM, HIDDEN_DIM)) * 0.1
    b1 = jnp.zeros(HIDDEN_DIM)
    w2 = random.normal(k2, (HIDDEN_DIM, HIDDEN_DIM)) * 0.1
    b2 = jnp.zeros(HIDDEN_DIM)
    w3 = random.normal(k3, (HIDDEN_DIM, OUTPUT_DIM)) * 0.1
    b3 = jnp.zeros(OUTPUT_DIM)
    return (w1, b1), (w2, b2), (w3, b3)

def model(params, coords):
    (w1, b1), (w2, b2), (w3, b3) = params
    h = jnp.dot(coords, w1) + b1
    h = jnp.tanh(h)
    h = jnp.dot(h, w2) + b2
    h = jnp.tanh(h)
    out = jnp.dot(h, w3) + b3
    return out * 5.0  # Scale to -5 to 5

# Generate coordinate grid (same as before)
x = jnp.linspace(0, 1, GRID_SIZE)
z = jnp.linspace(0, 1, GRID_SIZE)
X, Z = jnp.meshgrid(x, z)
coords = jnp.stack([X.flatten(), Z.flatten()], axis=1)  # Shape: (16384, 2)

# Synthetic target heightmap (mimicking your reference)
def create_target_heightmap():
    u = coords[:, 0]  # x-coordinates
    v = coords[:, 1]  # z-coordinates
    heights = jnp.sin(u * 6.28 * 2) * jnp.cos(v * 6.28 * 2) * 5.0  # -5 to 5
    return heights.reshape(GRID_SIZE, GRID_SIZE)

# Loss function
def loss_fn(params, coords, target):
    pred = model(params, coords)
    pred = pred.reshape(GRID_SIZE, GRID_SIZE)
    return jnp.mean((pred - target) ** 2)

# Training loop
def train_model(seed=42, num_steps=1000, learning_rate=0.01):
    # Initialize parameters and optimizer
    key = random.PRNGKey(seed)
    params = init_params(key)
    optimizer = optax.adam(learning_rate)
    opt_state = optimizer.init(params)

    # Get target heightmap
    target = create_target_heightmap()

    # Define update step
    @jax.jit
    def step(params, opt_state, coords, target):
        loss, grads = value_and_grad(loss_fn)(params, coords, target)
        updates, opt_state = optimizer.update(grads, opt_state)
        params = optax.apply_updates(params, updates)
        return params, opt_state, loss

    # Train
    print("Training started...")
    for step_idx in range(num_steps):
        params, opt_state, loss = step(params, opt_state, coords, target)
        if step_idx % 100 == 0:
            print(f"Step {step_idx}, Loss: {loss:.4f}")

    print("Training finished.")
    return params

# Save weights to binary file
def save_weights(params, filename="model_weights.bin"):
    (w1, b1), (w2, b2), (w3, b3) = params
    flat_params = jnp.concatenate([
        w1.flatten(), b1.flatten(),
        w2.flatten(), b2.flatten(),
        w3.flatten(), b3.flatten()
    ])
    flat_params.tofile(filename)
    print(f"Weights saved to {filename}")

# Main execution
if __name__ == "__main__":
    # Train the model
    trained_params = train_model(seed=42, num_steps=1000, learning_rate=0.01)

    # Generate and print some info about the result
    heightmap = model(trained_params, coords).reshape(GRID_SIZE, GRID_SIZE)
    print("Generated heightmap shape:", heightmap.shape)
    print("Min height:", jnp.min(heightmap))
    print("Max height:", jnp.max(heightmap))

    # Save the weights for C integration
    save_weights(trained_params, "model_weights.bin")
