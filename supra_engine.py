import sys
import time
import chess

# === Engine Settings ===
uci_options = {
    "Threads": 1,
    "Hash": 128,
    "Move Overhead": 100,
    "Max Nodes": 0  # 0 = unlimited
}
MAX_DEPTH = 25
TIME_LIMIT = 3.0
transposition_table = {}
history_heuristic = {}
node_count = 0
NODE_LIMIT = None

PIECE_VALUES = {
    chess.PAWN: 100, chess.KNIGHT: 320, chess.BISHOP: 330,
    chess.ROOK: 500, chess.QUEEN: 900, chess.KING: 20000
}

PIECE_SQUARE_TABLES = {pt: [0]*64 for pt in PIECE_VALUES}
PIECE_SQUARE_TABLES[chess.PAWN] = [
    0, 0, 0, 0, 0, 0, 0, 0,
    5, 10, 10, -20, -20, 10, 10, 5,
    5, -5, -10, 0, 0, -10, -5, 5,
    0, 0, 0, 20, 20, 0, 0, 0,
    5, 5, 10, 25, 25, 10, 5, 5,
    10, 10, 20, 30, 30, 20, 10, 10,
    50, 50, 50, 50, 50, 50, 50, 50,
    0, 0, 0, 0, 0, 0, 0, 0
]

CENTER_SQUARES = [chess.D4, chess.E4, chess.D5, chess.E5]

def evaluate(board):
    if board.is_checkmate():
        return -999999 if board.turn else 999999
    if board.is_stalemate() or board.is_insufficient_material():
        return 0
    score = 0
    for pt in PIECE_VALUES:
        for sq in board.pieces(pt, chess.WHITE):
            score += PIECE_VALUES[pt] + PIECE_SQUARE_TABLES[pt][sq]
        for sq in board.pieces(pt, chess.BLACK):
            score -= PIECE_VALUES[pt] + PIECE_SQUARE_TABLES[pt][chess.square_mirror(sq)]
    for sq in CENTER_SQUARES:
        piece = board.piece_at(sq)
        if piece:
            score += 10 if piece.color == chess.WHITE else -10
    return score if board.turn == chess.WHITE else -score

def is_quiet(board, move):
    return not board.is_capture(move) and not board.gives_check(move)

def sort_moves(board, moves):
    def move_score(move):
        score = 0
        if board.is_capture(move):
            captured = board.piece_at(move.to_square)
            if captured:
                score += 10_000 + PIECE_VALUES[captured.piece_type]
        if board.gives_check(move):
            score += 500
        score += history_heuristic.get((board.turn, move.uci()), 0)
        return -score
    return sorted(moves, key=move_score)

def quiescence(board, alpha, beta):
    global node_count
    node_count += 1
    if NODE_LIMIT and node_count > NODE_LIMIT:
        raise TimeoutError
    stand_pat = evaluate(board)
    if stand_pat >= beta:
        return beta
    if alpha < stand_pat:
        alpha = stand_pat
    for move in board.legal_moves:
        if board.is_capture(move):
            board.push(move)
            score = -quiescence(board, -beta, -alpha)
            board.pop()
            if score >= beta:
                return beta
            if score > alpha:
                alpha = score
    return alpha

def alpha_beta(board, depth, alpha, beta, start_time, max_time, ply=0):
    global node_count
    if time.time() - start_time > max_time:
        raise TimeoutError
    if NODE_LIMIT and node_count > NODE_LIMIT:
        raise TimeoutError

    key = hash((board.board_fen(), board.turn, board.castling_rights, board.ep_square))
    if key in transposition_table and transposition_table[key]["depth"] >= depth:
        return transposition_table[key]["value"]

    if depth == 0:
        return quiescence(board, alpha, beta)

    legal_moves = list(board.legal_moves)
    if not legal_moves:
        return evaluate(board)

    value = -float('inf')
    ordered_moves = sort_moves(board, legal_moves)

    for i, move in enumerate(ordered_moves):
        board.push(move)
        node_count += 1
        if i >= 3 and depth >= 3 and is_quiet(board, move):
            reduced_depth = depth - 2
            score = -alpha_beta(board, reduced_depth, -beta, -alpha, start_time, max_time, ply+1)
        else:
            score = -alpha_beta(board, depth - 1, -beta, -alpha, start_time, max_time, ply+1)
        board.pop()

        if score > value:
            value = score
            if score > alpha:
                alpha = score
                history_heuristic[(board.turn, move.uci())] = history_heuristic.get((board.turn, move.uci()), 0) + depth**2
        if alpha >= beta:
            break  # beta cutoff

    transposition_table[key] = {"value": value, "depth": depth}
    return value

def choose_best_move(board, depth=None, nodes=None, time_limit=TIME_LIMIT):
    global node_count, NODE_LIMIT
    node_count = 0
    NODE_LIMIT = nodes
    best_move = None
    best_score = -float('inf')
    start_time = time.time()
    max_depth = depth if depth else MAX_DEPTH

    try:
        for d in range(1, max_depth + 1):
            for move in board.legal_moves:
                board.push(move)
                try:
                    score = -alpha_beta(board, d - 1, -float("inf"), float("inf"), start_time, time_limit)
                except TimeoutError:
                    board.pop()
                    raise
                board.pop()
                if score > best_score:
                    best_score = score
                    best_move = move
            if time.time() - start_time > time_limit:
                break
    except TimeoutError:
        pass

    return best_move

def handle_position(board, line):
    parts = line.split()
    if "startpos" in parts:
        board.reset()
        if "moves" in parts:
            for mv in parts[parts.index("moves") + 1:]:
                board.push_uci(mv)
    elif "fen" in parts:
        idx = parts.index("fen")
        fen = " ".join(parts[idx + 1:idx + 7])
        board.set_fen(fen)
        if "moves" in parts:
            for mv in parts[parts.index("moves") + 1:]:
                board.push_uci(mv)

def set_uci_option(line):
    parts = line.split()
    if "name" in parts and "value" in parts:
        name = " ".join(parts[parts.index("name") + 1:parts.index("value")])
        value = int(parts[parts.index("value") + 1])
        if name in uci_options:
            uci_options[name] = value

def handle_go(board, line):
    depth = None
    nodes = None
    parts = line.split()
    if "depth" in parts:
        depth = int(parts[parts.index("depth") + 1])
    if "nodes" in parts:
        nodes = int(parts[parts.index("nodes") + 1])
    move = choose_best_move(board, depth=depth, nodes=nodes)
    if move:
        print("bestmove", move.uci())
    else:
        print("bestmove 0000")

def main():
    board = chess.Board()
    while True:
        line = sys.stdin.readline()
        if not line:
            break
        line = line.strip()
        if line == "uci":
            print("id name Supracheckmate v2")
            print("id author Suprateem Das")
            print("option name Threads type spin default 1 min 1 max 16")
            print("option name Hash type spin default 128 min 1 max 4096")
            print("option name Move Overhead type spin default 100 min 0 max 10000")
            print("option name Max Nodes type spin default 0 min 0 max 100000000")
            print("uciok")
        elif line == "isready":
            print("readyok")
        elif line.startswith("setoption"):
            set_uci_option(line)
        elif line.startswith("ucinewgame"):
            board.reset()
        elif line.startswith("position"):
            handle_position(board, line)
        elif line.startswith("go"):
            handle_go(board, line)
        elif line == "quit":
            break
        sys.stdout.flush()

if __name__ == "__main__":
    main()
