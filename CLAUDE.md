# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

The Gameplay Work Balancer (GWB) is an Unreal Engine plugin that helps distribute and balance intensive computational work across multiple frames to maintain performance targets. It's designed to prevent gameplay hitches by managing the execution time of background tasks, AI processing, and other computationally intensive systems.

## Architecture

The GWB system consists of the following key components:

1. **GWBManager** - Central manager responsible for scheduling and executing work units, maintaining work groups, and handling the lifecycle of work units.

2. **GWBSubsystem** - Engine subsystem that initializes the GWB system and provides access to components like TimeSlicers.

3. **GWBScheduler** - Controls when work cycles start and manages the timing for executing work.

4. **GWBTimeSlicer** - Manages time budget allocation and tracking for work execution, ensuring work doesn't exceed frame budgets.

5. **GWBWorkUnit** - Represents a unit of work to be executed, containing callback delegates and state tracking.

6. **GWBWorkUnitHandle** - Handle returned to callers providing an interface to interact with scheduled work units.

The workflow typically involves:
- Game code schedules work through the GWBManager
- Work is organized into work groups with specific budget constraints
- The scheduler determines when work should be executed
- Time slicers enforce budget constraints to prevent excessive frame time usage

Tests are organized into:
- GWBManagerTests - Tests for the core manager functionality
- GWBSchedulerTests - Tests for the scheduler behavior

## Development

### Console Variables (CVars)

The plugin behavior can be configured with the following console variables:

- `gwb.enabled` - Whether the balancer is enabled (default: true)
- `gwb.frame.budget` - Time in seconds balancer may spend per frame (default: 0.005)
- `gwb.frame.interval` - Time in seconds between balancer work frames (default: 0.0 = every frame)
- `gwb.escalation.scalar` - Maximum offset scalar to frame budget when escalation triggered (default: 0.5)
- `gwb.escalation.count` - Number of work instances for escalation reference (default: 30)
- `gwb.escalation.duration` - How quickly escalation should scale up in seconds (default: 0.5)
- `gwb.escalation.decay` - How quickly escalation should scale down in seconds (default: 0.5)
- `gwb.immediateduringwork` - Whether work in current category is immediately executed (default: true)

When testing specific behavior, these variables can be modified at runtime or overridden in tests using the `FScopedCVarOverride` utility.

### Test Utilities

When writing tests for GWB functionality:

1. Use `FScopedCVarOverride` to temporarily change console variables during tests
2. Create mock objects by extending the real implementations and adding test-specific methods prefixed with `TEST_`
3. Use helper classes like `FGWBManagerTestHelper` for common test setup
4. Test both normal and edge cases (zero budget, exceeding budget, etc.)