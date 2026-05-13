import os
import numpy as np
import matplotlib.pyplot as plt
import argparse
import matplotlib.animation as animation

def get_files(folder):
    files = [f for f in sorted(os.listdir(folder)) if f.endswith(".txt")]
    if len(files) == 0:
        print(f"No files found in {folder} directory")
        return None
    return files

def plot_orbits(files, folder, plot, labels, star_label, title):
    
    # Create high-resolution figure
    plt.figure(figsize=(10, 10), dpi=200)
    
    for i,f in enumerate(files):
        data = np.loadtxt(os.path.join(folder, f))
        # The file format is: t, x, y, vx, vy, ax, ay
        # We want to plot the trajectory, which is y vs x.
        # x is the 2nd column (index 1), y is the 3rd column (index 2)
        plot(data[:, 1], data[:, 2], label=labels[i])
    
    # Add a central point for the sun
    plt.plot(0, 0, 'o', color='gold', markersize=10, label=star_label)
    plt.gca().set_aspect('equal')
    plt.title(title)
    plt.legend(labels)
    plt.xlabel("x position")
    plt.ylabel("y position")
    plt.grid(True)
    plt.savefig(os.path.join(folder, "plot.png"), dpi=200, bbox_inches='tight')

def plot_gif(files, folder, labels, star_label, title):
    
    # Load all data
    all_data = [np.loadtxt(os.path.join(folder, f)) for f in files]
    num_frames = len(all_data[0])
    
    fig, ax = plt.subplots(figsize=(10, 10), dpi=100)
    
    # Determine axis limits from all data
    all_x = np.concatenate([d[:, 1] for d in all_data])
    all_y = np.concatenate([d[:, 2] for d in all_data])
    margin = 0.1
    x_range = max(abs(all_x.min()), abs(all_x.max())) * (1 + margin)
    y_range = max(abs(all_y.min()), abs(all_y.max())) * (1 + margin)
    limit = max(x_range, y_range)
    
    def animate(frame):
        ax.clear()
        ax.set_xlim(-limit, limit)
        ax.set_ylim(-limit, limit)
        ax.set_aspect('equal')
        ax.set_title(title)
        ax.set_xlabel("x position")
        ax.set_ylabel("y position")
        ax.grid(True)
        
        # Plot the sun/star
        ax.plot(0, 0, 'o', color='gold', markersize=10, label=star_label)
        
        # Plot trajectories up to current frame and current positions
        for i, data in enumerate(all_data):
            # Plot trail
            ax.plot(data[:frame+1, 1], data[:frame+1, 2], alpha=0.5, label=labels[i])
            # Plot current position
            ax.plot(data[frame, 1], data[frame, 2], 'o', markersize=6)
        
        ax.legend()
        return []
    
    # Create animation (use every 10th frame to reduce file size)
    step = max(1, num_frames // 100)
    frames = range(0, num_frames, step)
    anim = animation.FuncAnimation(fig, animate, frames=frames, interval=50, blit=True)
    
    # Save as GIF
    anim.save(os.path.join(folder, "plot.gif"), writer='pillow', fps=20)
    plt.close()

def main():
    path = os.path.dirname(os.path.abspath(__file__))
    # TODO: fix here ---------------------------------------------
    default_dir = os.path.join(path, "artifacts")
    # TODO: fix here ---------------------------------------------
    parser = argparse.ArgumentParser(description="Plot solar system or TRAPPIST-1 trajectories")
    parser.add_argument("--dir", type=str, default=default_dir, help="Directory containing data files")
    parser.add_argument("--type", type=str, choices=["scatter", "line"], default="line", help="Plot type")
    parser.add_argument("--format", type=str, choices=["png", "gif"], default="gif", help="Plot format")
    
    args = parser.parse_args()
    
    if args.type == "scatter":
        plot = lambda x, y, **kwargs: plt.scatter(x, y, s=1, **kwargs)
    else:
        plot = plt.plot
    
    folder = args.dir
        
    files = get_files(folder)
    
    # detect if Sun or trappist:
    data = np.loadtxt(os.path.join(folder, files[0]))
    if max(data[:, 1]) < 0.1:
        title = "TRAPPIST-1 System"
        labels = ["b", "c", "d", "e"]
        star_label = "TRAPPIST-1"
    else:
        title = "Solar System"
        labels = ["mercury", "venus", "earth", "mars"]
        star_label = "Sun"
    
    if args.format == "png":
        plot_orbits(files, folder, plot, labels, star_label, title)
    elif args.format == "gif":
        plot_gif(files, folder, labels, star_label, title)
    
if __name__ == "__main__":
    main()