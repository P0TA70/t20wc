import sys
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from datetime import datetime

def gantt_chart_visual(player_names):
    """Create a professional Gantt chart visualization with player names"""
    
    size = len(player_names)
    
    colors = ['#FF6B6B', '#4ECDC4', '#45B7D1', '#FFA07A', '#98D8C8',
              '#F7DC6F', '#BB8FCE', '#85C1E2', '#F8B88B', '#A9DFBF', '#D7DBDD']
    
    # Get unique players and their IDs
    unique_players = []
    player_to_id = {}
    
    for name in player_names:
        if name not in player_to_id:
            player_to_id[name] = len(unique_players)
            unique_players.append(name)
    
    num_players = len(unique_players)
    
    fig, ax = plt.subplots(figsize=(18, max(7, num_players * 0.6)))
    
    # Find continuous blocks
    process_times = {i: [] for i in range(num_players)}
    current_player = player_names[0]
    current_id = player_to_id[current_player]
    start_time = 0
    
    for time in range(1, size):
        if player_names[time] != current_player:
            process_times[current_id].append((start_time, time - start_time))
            current_player = player_names[time]
            current_id = player_to_id[current_player]
            start_time = time
    
    process_times[current_id].append((start_time, size - start_time))
    
    # Draw rectangles
    for player_id in range(num_players):
        for start, duration in process_times[player_id]:
            color = colors[player_id % len(colors)]
            rect = patches.Rectangle((start, player_id), duration, 0.8, 
                                     linewidth=2, edgecolor='#333333', 
                                     facecolor=color, alpha=0.85)
            ax.add_patch(rect)
            
            if duration > 0.5:
                ax.text(start + duration/2, player_id + 0.4, unique_players[player_id], 
                       ha='center', va='center', fontweight='bold', fontsize=9, color='white')
    
    ax.set_xlim(-0.5, size + 0.5)
    ax.set_ylim(-0.5, num_players)
    ax.set_xlabel('Time Units', fontsize=13, fontweight='bold')
    ax.set_ylabel('Players', fontsize=13, fontweight='bold')
    ax.set_title('Team Player Scheduling - Gantt Chart', fontsize=15, fontweight='bold', pad=20)
    ax.set_yticks(range(num_players))
    ax.set_yticklabels(unique_players)
    
    # SET X-AXIS TICKS TO INTERVALS OF 5
    ax.set_xticks(range(0, size + 1, 5))
    
    # ROTATE X-AXIS LABELS
    ax.tick_params(axis='x', rotation=45, labelsize=10)
    
    # Fine grid for every unit, but lighter
    ax.grid(axis='x', alpha=0.2, linestyle=':', linewidth=0.5)
    ax.set_axisbelow(True)
    
    # Create unique filename with timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f'gantt_chart_{timestamp}.png'
    
    plt.savefig(filename, dpi=150, bbox_inches='tight', facecolor='white')
    print(f"✓ Gantt chart saved as '{filename}'")
    plt.close()

if __name__ == "__main__":
    import sys
    # Get player names from command line arguments
    player_names = sys.argv[1:]
    
    if len(player_names) == 0:
        print("Error: No player names provided")
        sys.exit(1)
    
    gantt_chart_visual(player_names)