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
    all_data = [np.loadtxt(os.path.join(folder, f)) for f in files]
    num_frames = len(all_data[0])
    all_x = np.concatenate([d[:, 1] for d in all_data])
    all_y = np.concatenate([d[:, 2] for d in all_data])
    limit = max(abs(all_x).max(), abs(all_y).max()) * 1.1

    fig, ax = plt.subplots(figsize=(10, 10), dpi=150)
    ax.set(xlim=(-limit, limit), ylim=(-limit, limit), aspect='equal', title=title,
           xlabel="x position", ylabel="y position")
    ax.grid(True)
    ax.plot(0, 0, 'o', color='gold', markersize=10, label=star_label)
    trails = [ax.plot([], [], alpha=0.5, label=labels[i])[0] for i in range(len(all_data))]
    points = [ax.plot([], [], 'o', markersize=6)[0] for _ in all_data]
    ax.legend()

    def animate(frame):
        for i, data in enumerate(all_data):
            trails[i].set_data(data[:frame + 1, 1], data[:frame + 1, 2])
            points[i].set_data([data[frame, 1]], [data[frame, 2]])
        return trails + points

    step = max(1, num_frames // 100)
    anim = animation.FuncAnimation(fig, animate, frames=range(0, num_frames, step), blit=True)
    anim.save(os.path.join(folder, "plot.gif"), writer='pillow', fps=15, dpi=72)
    plt.close()

def main():
    path = os.path.dirname(os.path.abspath(__file__))
    default_dir = os.path.join(path, "artifacts", "solar_system")
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
    if files is None:
        print(f"No files found in {folder} directory")
        return
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