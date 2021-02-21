## 两阶段提交协议（Two Phase Commit Protocol）
### 两个角色
- Coordinator：主导者，可以和所有 Participant 通信
- Participant：被动执行

## Prepare Phase
数据库在本阶段不提交（commit）或者回滚（roll back）某个事务。而是 Coordinator 要求所有 Participant 准备提交（投票、承诺）：无论发生什么，要么 redo 要么 undo 一个事务。

## Commit Phase
Coordinator 一旦收到所有 Participant 的回复，Coordinator 做出如下裁决：
- 如果存在一个 Participant 拒绝，Coordinator 广播所有 Participant 进行 abort
- 如果所有 Participant 同意，Coordinator 同步将 commit 信息写入 log 文件，然后广播所有 Participant 提交，最后，所有 Participant 确认（ACK）信息到了后，Coordinator 结束。

## 示例
Case#1 成功提交
```
    COORDINATOR               PARTICIPANT
  commit
-------->     request commit
        --------------------------->
                agree
        <---------------------------
                commit
        --------------------------->
    yes                              commit
<-------                            ------->
```
Case#2 Participant 自己拒绝
```
    COORDINATOR               PARTICIPANT
  commit
-------->     request commit
        --------------------------->
                  abort
        <---------------------------
    no                               abort
<-------                            ------->
```
Case#3 其他 Participant 拒绝
```
    COORDINATOR               PARTICIPANT
  commit
-------->     request commit
        --------------------------->
                agree
        <---------------------------
                abort
        --------------------------->
    no                               abort
<-------                            ------->
```

## 伪代码
```
def Coordinator():
    vote = 'COMMIT' # collect votes
    for p in Participants:
        Send(p, REQUEST_COMMIT)
        vote = WaitReply(p)
        if vote == 'ABORT'
            break

    if vote == 'COMMIT': # all agree then commit
        WriteLOGAndFlush(PHASE12_COMMIT)
        for p in Participants:
            Send(p, COMMIT)
            WaitACK(p)
            if Timeout():
                Retry()
    else: # any abort
        for p in Participants:
            Send(p, ABORT)
            WaitACK(p)
            if Timeout():
                Retry()
    WriteLOGAndFlush(COORDINATOR_COMPLETE)
    return

def Participant():
    WaitRequest(req)
    FlushUndoAndRedoLog()
    if Sucess(req):
        ReplyToCoordinator('AGREE')
    else:
        ReplyToCoordinator('ABORT')
    WaitVerdict(vote)
    if vote == 'COMMIT':
        ReleaseResource()
    else:
        Undo()
    ReplyACK()
```

## 数据恢复
如果 Coordinator 在 PHASE12_COMMIT 之前崩溃（日志中不存在 PHASE12_COMMIT 信息），Coordinator 重启后将 ABORT 广播给所有 Participant；如果 PAHSE12_COMMIT 已经保存在日志中但是 COORDINATOR_COMPLETE 丢失，Coordinator 重启后重新将 COMMIT 广播给有所 Participant；如果 COORDINATOR_COMPLETE 保存在日志中，Coordinator 重启时忽略上次事务。

同样的，如果上一个事务（在日志中）没有被标记为 AGREE，并且如果上一个事务被标记为 AGREE，Participant 重启后，会询问 Coordinator 该事务是否 commit 或者 abort，根据回复进行 redo 或者 undo。