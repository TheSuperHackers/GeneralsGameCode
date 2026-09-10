export module Graphics.Frame.SubmissionStatistics;

import Graphics.RHI;

namespace Graphics {

// Counts are cumulative on a command list. A display iteration snapshots its
// initial totals before any offscreen work and publishes the delta only after
// presentation succeeds. Source identity prevents combining command lists.
export class FrameSubmissionStatistics final
{
public:
    bool Begin(const void* source, RHISubmissionCounts totals) noexcept
    {
        if (source == nullptr || m_source != nullptr)
            return false;
        m_source = source;
        m_start = totals;
        return true;
    }

    bool Complete(const void* source, RHISubmissionCounts totals) noexcept
    {
        if (m_source == nullptr || source != m_source
            || totals.draw_calls < m_start.draw_calls
            || totals.triangles < m_start.triangles
            || totals.vertex_invocations < m_start.vertex_invocations)
            return false;
        m_last = {totals.draw_calls - m_start.draw_calls,
            totals.triangles - m_start.triangles,
            totals.vertex_invocations - m_start.vertex_invocations};
        Cancel();
        return true;
    }

    void Cancel() noexcept
    {
        m_source = nullptr;
        m_start = {};
    }

    void Reset() noexcept
    {
        Cancel();
        m_last = {};
    }

    RHISubmissionCounts Last_Frame() const noexcept { return m_last; }

private:
    const void* m_source = nullptr;
    RHISubmissionCounts m_start{};
    RHISubmissionCounts m_last{};
};

export FrameSubmissionStatistics& Get_Frame_Submission_Statistics() noexcept
{
    static FrameSubmissionStatistics statistics;
    return statistics;
}

}
